#pragma once
#include "Transform.h"
#include <cmath>
#include "Vector.h"
#define PI 3.141592

#include "Renderer.h"
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

	FMatrix GetProjectionMatrix(float Aspect, float fovDegree, float n, float f)
	{
		//fov 단위는 라디안
		//Aspect = width/height
		FMatrix result = FMatrix::Zero; //영벡터
		float yScale = 1.0f / tanf((fovDegree / 2)*PI/180); //xScale
		float xScale = yScale / Aspect;


		result.M[0][0] = xScale; //xScale
		result.M[1][1] = yScale; //yScale
		result.M[2][2] = f / (f - n); //A 임시
		result.M[3][2] = -n * f / (f - n); //B 임시
		result.M[2][3] = 1;

		return result;
	}

	void Rotate(long Dx, long Dy)
	{
		Transform.Rotation.Yaw += Dx * Sensitivity;
		Transform.Rotation.Pitch -= Dy * Sensitivity;
	}


	void SetSensitivity(float _v) { Sensitivity = _v; }

private:
	float Sensitivity = 0.1f;
};
