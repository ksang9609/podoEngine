#pragma once
#include "Matrix.h"
#include "MathUtility.h"

template<typename T>
struct TRotator
{
	T Pitch, Yaw, Roll;

	TRotator() = default;

	TRotator(T InPitch, T InYaw, T InRoll)
		: Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}

	//static const TRotator ZeroRotator = { 0, 0, 0 };

	const FMatrix ToMatrix() const
	{
		//Pitch, Yaw, Roll의 각각 cossin 구하기
		FMatrix Matrix = FMatrix::Zero;
		T cosP, cosY, cosR;
		T sinP, sinY, sinR;

		//도 -> 라디안 변환 후 sincos 호출
		FMath::sincos<T>(sinP, cosP, Pitch * PI / 180);
		FMath::sincos<T>(sinY, cosY, Yaw * PI / 180);
		FMath::sincos<T>(sinR, cosR, Roll * PI / 180);

		Matrix.M[0][0] = cosP * cosY;
		Matrix.M[0][1] = cosP * sinY;
		Matrix.M[0][2] = sinP;
		Matrix.M[1][0] = sinR * sinP * cosY - cosR * sinY;
		Matrix.M[1][1] = sinR * sinP * sinY + cosR * cosY;
		Matrix.M[1][2] = -sinR * cosP;
		Matrix.M[2][0] = -(cosR * sinP * cosY + sinR * sinY);
		Matrix.M[2][1] = sinR * cosY - cosR * sinP * sinY;
		Matrix.M[2][2] = cosR * cosP;
	}
};

using FRotator = TRotator<float>;
