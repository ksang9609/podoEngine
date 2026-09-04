#include "Ball.h"

void UBall::Move(float deltaTime)
{
	Transform.Location.x += Velocity.x * deltaTime;
	Transform.Location.y += Velocity.y * deltaTime;
	Transform.Location.z += Velocity.z * deltaTime;
}

void UBall::SetRadius(float newRadius)
{
	Transform.Scale = newRadius;
}
