#include "FEditorViewportClient.h"

#include "Cube.h"
#include "Sphere.h"
#include "WindowApplication.h"
#include "ImGui/imgui.h"

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

	float NearlistT = FLT_MAX;

	if (true) //if(도형이 선택되었다면)
	{
		mGizmo.mbVisible = true;
		// Gizmo 탐색
		if (mGizmo.IsRayInGizmo(NearPoint, FarPoint))
		{
			// gizmo highlight
			return;
		}
	}
	else
	{
		mGizmo.mbVisible = false;
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
				HoveredRenderInfo = RI;
				if (RI.ObejctID.UUID == ClickedRenderInfo.ObejctID.UUID)
				{
					ClickedRenderInfo = RI;
					ClickedActor = UObject::GetObjectByInternalIndex<AActor>(RI.ObejctID.InternalIndex);
				}
			}
		}
	}
}

void FEditorViewportClient::Update(float deltaTime)
{
	const FInputState& Input = WindowApplication.Input;
	ImGuiIO& io = ImGui::GetIO();

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

	mGizmo.Update(mCamera.Transform.Location, mCamera.GetForwardVector(), mCamera.mFovDegree);
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
