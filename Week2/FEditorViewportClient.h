#pragma once

#include "Camera.h"

struct FEditorViewportClient
{
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
