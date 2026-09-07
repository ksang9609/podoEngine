#pragma once
#include "Vector.h"

#include "Camera.h"

struct FEditorViewportClient
{

	bool RayIntersectsTriangle(
		const FVector& Origin,
		const FVector& Dir,
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		float& OutT, float& OutU, float& OutV) {

		const float EPSILON = 1e-6f;

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
		\
			OutU = FVector::dot(T, P) * InvDet;
		if (OutU < 0.0f || OutU > 1.0f) return false;

		FVector Q = FVector::cross(T, E1);
		OutV = FVector::dot(D, Q) * InvDet;
		if (OutV < 0.0f || OutU + OutV > 1.0f) return false;

		OutT = FVector::dot(E2, Q) * InvDet;

		return (OutT > EPSILON);                  // 광선 앞쪽만

	}

	void DeprojectScreenToWorld(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH,
		const FCamera& Cam,
		float FovDegree, float NearZ, float FarZ,
		FVector& OutNearPoint, FVector& OutFarPoint)
	{
		// 1) 픽셀 -> NDC. 화면 Y 는 아래로 +, NDC Y 는 위로 + 라서 뒤집는다
		const float ndcX = (2.0f * (MouseX + 0.5f) / ScreenW) - 1.0f;
		const float ndcY = 1.0f - (2.0f * (MouseY + 0.5f) / ScreenH);

		// 2) 투영 스케일 항 — GetProjectionMatrix 와 반드시 같은 식이어야 한다
		const float Aspect = ScreenW / ScreenH;
		const float yScale = 1.0f / tanf(FovDegree * 0.5f * PI / 180.f);
		const float xScale = yScale / Aspect;

		// 3) 카메라 기저로 월드 방향 합성. 전방 성분이 1 이므로 정규화하면 안 된다
		const FMatrix R = FMatrix::Rotate(Cam.Transform.Rotation);
		FVector V = R.GetUnitAxis(EAxis::X);                    // 전방 (성분 1)
		V += R.GetUnitAxis(EAxis::Y) * (ndcX / xScale);         // 우측
		V += R.GetUnitAxis(EAxis::Z) * (ndcY / yScale);         // 상방

		// 4) 곱하면 그대로 각 평면 위의 점
		OutNearPoint = Cam.Transform.Location + V * NearZ;
		OutFarPoint = Cam.Transform.Location + V * FarZ;
	}
};
