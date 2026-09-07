#include "FEditorViewportClient.h"

#include "Cube.h"
#include "Sphere.h"
#include "WindowApplication.h"
#include "ImGui/imgui.h"

void FEditorViewportClient::RayCast(D3D11_VIEWPORT ViewportInfo, UWorld* World)
{
	bMouseHit = false;

	FVector NearPoint, OutPoint;
	DeprojectScreenToWorld(WindowApplication.Input.CursorX, WindowApplication.Input.CursorY,
		ViewportInfo.Width, ViewportInfo.Height, 0.1f, 100.f, NearPoint, OutPoint);

	float OutT, OutU, OutV;
	float NearlistT = (OutPoint - NearPoint).Length();

	const TArray<FRenderInfo> RenderInfos = World->GetRenderInfos();
	FVertexSimple* vertices = nullptr;
	int length = 0;
	for (const FRenderInfo& RI : RenderInfos)
	{
		bool bHit = false;
		if (RI.ePrimitive == EPrimitive::EP_Cube)
		{
			vertices = Cube_vertices;
			length = 36;
		}
		else if (RI.ePrimitive == EPrimitive::EP_Sphere)
		{
			vertices = Sphere_vertices;
			length = 2400;
		}
		else assert(!vertices && "Actor's RenderInfo.ePrimitive is NOT Valid");

		length = sizeof(*vertices) / sizeof(FVertexSimple);
		for (int i = 0; i < length - 2; i += 3)
		{
			FVector V0 = vertices[i].GetPosition(), V1 = vertices[i + 1].GetPosition(), V2 = vertices[i + 2].GetPosition();
			if (RayIntersectsTriangle(NearPoint, OutPoint, V0, V1, V2, OutT, OutU, OutV))
			{
				bHit = true;
				break;
			}
		}

		if (bHit && OutT < NearlistT)
		{
			NearlistT = OutT;
			bMouseHit = true;
			HoveredRenderInfo = RI;
			//if (RI.ObejctID.)
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
