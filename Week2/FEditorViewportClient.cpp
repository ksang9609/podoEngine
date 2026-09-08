#include "FEditorViewportClient.h"

#include "Cube.h"
#include "Sphere.h"
#include "WindowApplication.h"
#include "ImGui/imgui.h"
#include "Console.h"
#include "SceneManager.h"

// 정점 배열이 보이는 스코프라 sizeof 로 개수가 나온다.
// 포인터로 받으면 배열 크기 정보가 사라지므로 여기서 개수를 같이 넘긴다.
static bool GetPrimitiveMesh(EPrimitive ePrimitive, const FVertexSimple*& OutVertices, uint32& OutCount)
{
	switch (ePrimitive)
	{
	case EPrimitive::EP_Cube:
		OutVertices = Cube_vertices;
		OutCount = static_cast<uint32>(sizeof(Cube_vertices) / sizeof(FVertexSimple));
		return true;

	case EPrimitive::EP_Sphere:
		OutVertices = Sphere_vertices;
		OutCount = static_cast<uint32>(sizeof(Sphere_vertices) / sizeof(FVertexSimple));
		return true;
	}

	return false;
}

void FEditorViewportClient::RayCast(D3D11_VIEWPORT ViewportInfo, UWorld* World)
{
	bMouseHit = false;

	FVector NearPoint, FarPoint;
	DeprojectScreenToWorld(WindowApplication.Input.CursorX, WindowApplication.Input.CursorY,
		ViewportInfo.Width, ViewportInfo.Height, 0.1f, 100.f, NearPoint, FarPoint);

	mRayNear = NearPoint;
	mRayFar = FarPoint;

	float NearlistT = FLT_MAX;

	// 드래그 중에는 히트 판정을 하지 않는다.
	// 빠르게 끌면 커서가 축 캡슐을 벗어나는데, 그때 eAxis가 NONE이 되면 드래그가 끊긴다.
	if (mGizmo.mDraggingAxis != FGizmo::EGIZMO_AXIS::NONE)
	{
		bMouseHit = true;
		mGizmo.mbHovered = true;
		mGizmo.eAxis = mGizmo.mDraggingAxis;   // 끌고 있는 축의 강조를 유지한다
		return;
	}

	// Gizmo 탐색
	if (mGizmo.IsRayInGizmo(NearPoint, FarPoint))
	{
		bMouseHit = true;
		mGizmo.mbHovered = true;
		// gizmo highlight
		return;
	}

	// Object 탐색
	const TArray<FRenderInfo> RenderInfos = World->GetRenderInfos();
	for (const FRenderInfo& RI : RenderInfos)
	{
		const FVertexSimple* vertices = nullptr;
		uint32 length = 0;
		if (!GetPrimitiveMesh(RI.ePrimitive, vertices, length))
		{
			continue;   // 모르는 프리미티브는 건너뛴다
		}

		const FMatrix WorldToLocal = RI.WorldTransformMatrix.Inverse();
		const FVector LocalNear = WorldToLocal.TransformPosition(NearPoint);
		const FVector LocalFar = WorldToLocal.TransformPosition(FarPoint);

		// 삼각형 리스트라 정점 3개씩 묶인다
		for (uint32 i = 0; i + 2 < length; i += 3)
		{
			const FVector V0 = vertices[i].GetPosition();
			const FVector V1 = vertices[i + 1].GetPosition();
			const FVector V2 = vertices[i + 2].GetPosition();

			float OutT, OutU, OutV;
			if (RayIntersectsTriangle(LocalNear, LocalFar, V0, V1, V2, OutT, OutU, OutV)
				&& OutT < NearlistT)
			{
				// 같은 메시 안에서도 더 가까운 삼각형이 뒤에 나올 수 있으므로 break 하지 않는다
				NearlistT = OutT;
				bMouseHit = true;
				mHoveredRenderInfo = RI;
			}
		}
	}
}

void FEditorViewportClient::Update(float deltaTime, D3D11_VIEWPORT ViewportInfo, FSceneManager* sceneManager)
{
	const FInputState& Input = WindowApplication.Input;
	ImGuiIO& io = ImGui::GetIO();

	// Camera Transform
	// 회전을 이동보다 먼저 — 이번 프레임에 돌린 방향으로 바로 움직이게
	if (!io.WantCaptureMouse && Input.IsDown(VK_RBUTTON))
	{
		mCamera.Rotate(Input.MouseDX, Input.MouseDY);
	}

	if (!io.WantCaptureMouse && Input.MouseWheelDelta != 0.0f)
	{
		mCamera.Speed *= FMath::Pow(1.2f, Input.MouseWheelDelta);
		mCamera.Speed = FMath::Clamp(mCamera.Speed, 0.1f, 100.0f);
	}

	if (!io.WantCaptureKeyboard)
	{
		const FMatrix R = FMatrix::Rotate(mCamera.Transform.Rotation);
		const FVector Forward = R.GetUnitAxis(EAxis::X);
		const FVector Right = R.GetUnitAxis(EAxis::Y);

		FVector MoveInput(0.f, 0.f, 0.f);
		if (Input.IsDown('W')) MoveInput += Forward;
		if (Input.IsDown('S')) MoveInput -= Forward;
		if (Input.IsDown('D')) MoveInput += Right;
		if (Input.IsDown('A')) MoveInput -= Right;
		if (Input.IsDown('E')) MoveInput += FVector(0.f, 0.f, 1.f);
		if (Input.IsDown('Q')) MoveInput -= FVector(0.f, 0.f, 1.f);

		if (MoveInput.Length() > SMALL_NUMBER)
		{
			MoveInput.Normalize();
			mCamera.Velocity = MoveInput * mCamera.Speed;
			mCamera.Transform.Location += mCamera.Velocity * deltaTime;
		}
	}

	//Gizmo Test
	//mGizmo.mbVisible = true;
	//mGizmo.mLocation = { 0.0f, 2.0f, 0.0f };

	RayCast(ViewportInfo, sceneManager->GetCurrentWorld());

	//RayCast

	////Editor Click 처리
	//if (mClickedActor)
	//{
	//	mClickedActor->BeginFrame();
	//}

	// 누른 순간에만 선택을 갱신한다. 떼는 것으로는 선택이 풀리지 않는다.
	if (!ImGui::GetIO().WantCaptureMouse && Input.WasPressed(VK_LBUTTON))
	{
		AActor* Hit = nullptr;

		if (IsMouseHit())
		{
			//Gizmo라면 드래그 기준값을 저장
			if (mGizmo.eAxis != FGizmo::EGIZMO_AXIS::NONE &&
				sceneManager->IsActorSelected() &&
				mGizmo.mDraggingAxis == FGizmo::EGIZMO_AXIS::NONE)
			{
				mGizmo.BeginDrag(mRayNear, mRayFar, sceneManager->GetSelectedActor()->GetTransform());
			}

			//Actor라면 액터를 저장
			else
			{
				uint32 clickedObjectIndex = mHoveredRenderInfo.ObejctID.InternalIndex;
				UObject* ClickedObject = UObject::GetObjectByInternalIndex(clickedObjectIndex);
				if (ClickedObject && ClickedObject->IsA(AActor::GetClass()))
				{
					Hit = static_cast<AActor*>(ClickedObject);
				}
			}
		}

		//// 다른 것을 눌렀으면 이전 선택 해제. 같은 것이면 유지.
		//if (mClickedActor && mClickedActor != Hit && !mGizmo.mbHovered)
		//{
		//
		//	mClickedActor->UnPressed();
		//}

		//Gizmo를 제외한 다른 것을 눌렀을 때, ClickedActor로 갱신
		if (!mGizmo.mbHovered)
		{
			if (Hit != nullptr)
			{
				sceneManager->SetSelectedActor(Hit);
			}
			else
			{
				sceneManager->ResetSelectedActor();
			}
		}

		//if (Hit)
		//{
		//	Hit->Pressed();      // 선택 유지
		//	Hit->ClickStart();   // 이번 프레임에 시작했음을 표시
		//}
	}

	//Gizmo 축을 클릭한 상태로 마우스 이동이 있으면 해당 축 방향으로 ClickedActor을 변형한다.
	if (mGizmo.mDraggingAxis != FGizmo::EGIZMO_AXIS::NONE && sceneManager->IsActorSelected())
	{
		if (mGizmo.eType == FGizmo::EGIZMO_TYPE::TRANSLATE)
		{
			// 절대 좌표가 아니라 시작 시점 대비 변위. 축 직선도 시작 시점에 고정돼 있다
			FVector newLocation;
			if (mGizmo.GetDragLocation(mRayNear, mRayFar, newLocation))
			{
				sceneManager->GetSelectedActor()->SetLocation(newLocation);
			}
		}
		if (mGizmo.eType == FGizmo::EGIZMO_TYPE::ROTATE)
		{
			// 링 평면 위에서 잰 각도. 시작 회전에 누적각을 한 번만 얹는다
			FRotator newRotation;
			if (mGizmo.GetDragRotation(mRayNear, mRayFar, newRotation))
			{
				//ClickedActor->SetRotation(newRotation);
				mGizmo.UpdateRotation = newRotation;
				sceneManager->GetSelectedActor()->SetRotation(newRotation);
			}
		}
		if (mGizmo.eType == FGizmo::EGIZMO_TYPE::SCALE)
		{
			FVector newScale;
			if (mGizmo.GetDragScale(mRayNear, mRayFar, newScale))
			{
				sceneManager->GetSelectedActor()->SetScale(newScale);
			}
		}
	}

	if (!ImGui::GetIO().WantCaptureMouse && Input.WasReleased(VK_LBUTTON))
	{
		mGizmo.mDraggingAxis = FGizmo::EGIZMO_AXIS::NONE;
	}

	//변형된 Actor를 바탕으로 Gizmo를 위치시킨다.
	mGizmo.Update(sceneManager->GetSelectedActor(), mCamera.Transform.Location, mCamera.GetForwardVector(), mCamera.mFovDegree);

}

bool FEditorViewportClient::RayIntersectsTriangle(const FVector& Origin, const FVector& Dir, const FVector& V0, const FVector& V1, const FVector& V2, float& OutT, float& OutU, float& OutV)
{
	static const float EPSILON = 1e-6f;

	//삼각형판정 => O +tD = V0+ uE1+vE2
	// -tD + uE1 + vE2 = O - V0
	//E2=v2-v0. E1=v1-v0

	FVector D = Dir - Origin;
	FVector T = Origin - V0;
	FVector E2 = V2 - V0;
	FVector E1 = V1 - V0;
	FVector P = FVector::cross(D, E2);
	float Det = FVector::dot(E1, P);

	if (fabsf(Det) < EPSILON) return false;   // 평면과 평행

	float InvDet = 1.0f / Det;

	OutU = FVector::dot(T, P) * InvDet;
	if (OutU < 0.0f || OutU > 1.0f) return false;

	FVector Q = FVector::cross(T, E1);
	OutV = FVector::dot(D, Q) * InvDet;
	if (OutV < 0.0f || OutU + OutV > 1.0f) return false;

	OutT = FVector::dot(E2, Q) * InvDet;

	return (OutT > EPSILON);                  // 광선 앞쪽만

	// OutT : 맞은물체가 얼마나 가까이있나(float)
	// OutU, OutV 정확환 클릭지점을 확인하려면 필요
}

void FEditorViewportClient::DeprojectScreenToWorld(int32 MouseX, int32 MouseY, float ScreenW, float ScreenH, float NearZ, float FarZ, FVector& OutNearPoint, FVector& OutFarPoint)
{
	// 1) 픽셀 -> NDC. 화면 Y 는 아래로 +, NDC Y 는 위로 + 라서 뒤집는다
	const float ndcX = (2.0f * (MouseX + 0.5f) / ScreenW) - 1.0f;
	const float ndcY = 1.0f - (2.0f * (MouseY + 0.5f) / ScreenH);

	// 2) 투영 스케일 항 — GetProjectionMatrix 와 반드시 같은 식이어야 한다
	const float Aspect = ScreenW / ScreenH;
	const float yScale = 1.0f / tanf(mCamera.mFovDegree * 0.5f * PI / 180.f);
	const float xScale = yScale / Aspect;

	// 3) 카메라 기저로 월드 방향 합성. 전방 성분이 1 이므로 정규화하면 안 된다
	const FMatrix R = FMatrix::Rotate(mCamera.Transform.Rotation);
	FVector V = R.GetUnitAxis(EAxis::X);                    // 전방 (성분 1)
	V += R.GetUnitAxis(EAxis::Y) * (ndcX / xScale);         // 우측
	V += R.GetUnitAxis(EAxis::Z) * (ndcY / yScale);         // 상방

	// 4) 곱하면 그대로 각 평면 위의 점
	OutNearPoint = mCamera.Transform.Location + V * NearZ;
	OutFarPoint = mCamera.Transform.Location + V * FarZ;
}

void FEditorViewportClient::Reset()
{
	mHoveredRenderInfo = FRenderInfo();
	bMouseHit = false;
	mGizmo.Reset();
}
