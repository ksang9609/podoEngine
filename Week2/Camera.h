#pragma once
#include "Transform.h"
#include <cmath>
#include "Vector.h"

class FCamera
{
public:
	FCamera() : Transform(FTransform({ -2.0f, 1.0f, 1.0f }, { 0, 30, 0 }, { 1, 1, 1 }))
	{
		LookAt({ 0, 0, 0 });
	}

	FCamera(FTransform _FTransform) : Transform(_FTransform) {}
	FTransform Transform;

	FMatrix GetViewMatrix() const
	{
		// 카메라에는 스케일이 없다. 위치를 되돌리고, 회전을 되돌리고, 축을 교환한다.
		return FMatrix::Translation(FVector(-Transform.Location.x,
			-Transform.Location.y,
			-Transform.Location.z))
			* FMatrix::Rotate(Transform.Rotation).Transpose()
			* FMatrix::UEToDX;
	}

	// 특정 지점을 바라보도록 회전을 맞춘다.
	void LookAt(const FVector& Target)
	{
		Transform.Rotation = FRotator::LookAt(Transform.Location, Target);
	}

	FMatrix GetProjectionMatrix(float Aspect, float fovDegree, float n, float f) const
	{
		//fov 단위는 라디안
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

	FMatrix GetOrthographicMatrix(float width, float height, float n, float f) const
	{
		FMatrix result = FMatrix::Zero; // 영벡터

		result.M[0][0] = 2.0f / width;
		result.M[1][1] = 2.0f / height;
		result.M[2][2] = 1.0f / (f - n);
		result.M[3][2] = -n / (f - n);
		result.M[3][3] = 1.0f;

		return result;
	}

	void Rotate(long Dx, long Dy)
	{
		Transform.Rotation.Yaw += FMath::Fmod(Dx * Sensitivity, 360.f);
		Transform.Rotation.Pitch -= FMath::Fmod(Dy * Sensitivity, 360.f);
	}

	void Update();

	void SetSensitivity(float _v) { Sensitivity = _v; }
	FVector GetForwardVector() const { return FMatrix::Rotate(Transform.Rotation).GetUnitAxis(EAxis::X); }
	FVector GetRightVector()   const { return FMatrix::Rotate(Transform.Rotation).GetUnitAxis(EAxis::Y); }
	FVector GetUpVector()      const { return FMatrix::Rotate(Transform.Rotation).GetUnitAxis(EAxis::Z); }

	float Speed = 5.f;
	FVector Velocity = FVector(0);
	float Sensitivity = 0.1f;
	float mFovDegree = 60.f;
};
