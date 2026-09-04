#include "Ball.h"

void UBall::Move(float deltaTime)
{
	Location.x += Velocity.x * deltaTime;
	Location.y += Velocity.y * deltaTime;
	Location.z += Velocity.z * deltaTime;
}

bool UBall::Compact(const UPrimitive* Others) const
{
	const UBall* ball = static_cast<const UBall*>(Others);
	FVector v = ball->Location - Location;

	float Length = v.x * v.x + v.y * v.y + v.z * v.z;
	return Length < (Radius + ball->Radius) * (Radius + ball->Radius);
}

void UBall::SetRadius(float newRadius)
{
	Radius = newRadius;

	Mass = Radius * Radius * Radius * 1000.f;
}