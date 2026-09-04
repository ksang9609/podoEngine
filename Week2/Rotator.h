#pragma once
struct TRotator
{
	float Pitch, Yaw, Roll;

	TRotator() = default;

	TRotator(float InPitch, float InYaw, float InRoll)
		: Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}

	//static const TRotator ZeroRotator = { 0, 0, 0 };
};

using FRotator = TRotator;
