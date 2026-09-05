#pragma once
#include <cmath>
#include "Vector.h"
#include "MathUtility.h"

struct TRotator
{
	float Pitch, Yaw, Roll;

	TRotator() = default;

	TRotator(float InPitch, float InYaw, float InRoll)
		: Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}

	//static const TRotator ZeroRotator = { 0, 0, 0 };

	static TRotator FromDirection(const FVector& Direction)
	{
		// asin(z) 대신 atan2 를 쓴다. 정규화가 미세하게 어긋나도 NaN 이 나지 않는다.
		float Yaw = std::atan2(Direction.y, Direction.x);
		float Pitch = std::atan2(Direction.z, std::sqrt(Direction.x * Direction.x
													  + Direction.y * Direction.y));

		return TRotator(Pitch * 180 / PI, Yaw * 180 / PI, 0.0f);
	}

	// From 위치에서 To 위치를 바라보는 회전값.
	static TRotator LookAt(const FVector& From, const FVector& To)
	{
		return FromDirection(To - From);
	}

};

using FRotator = TRotator;
