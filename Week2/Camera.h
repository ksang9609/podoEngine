#pragma once
#include "Transform.h"

class FCamera
{
public:
	FCamera(FTransform _FTransform) : Transform(_FTransform) {}
	FTransform Transform;

	// 뷰 행렬 = 카메라 월드 행렬의 역행렬 * 축 교환
	//
	//   InverseMatrix() 까지는 좌표가 '언리얼 카메라 공간'(X 전방 / Y 우측 / Z 상방)에 있다.
	//   DirectX NDC 는 X 우측 / Y 위 / Z 안쪽 이므로 마지막에 축을 갈아끼워야
	//   카메라 정면에 있는 물체가 화면 중앙에 온다.
	FMatrix GetViewMatrix() const
	{
		return Transform.InverseMatrix() * FMatrix::UEToDX;
	}

	// 특정 지점을 바라보도록 회전을 맞춘다.
	void LookAt(const FVector& Target)
	{
		Transform.Rotation = FRotator::LookAt(Transform.Location, Target);
	}
};
