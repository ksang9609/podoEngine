#pragma once
#include "Vector.h"
#include "Transform.h"

class UPrimitive
{
public:
	UPrimitive(FTransform _Transform) : Transform(_Transform) {}
	virtual ~UPrimitive(){}

	virtual void Move(float deltaTime) = 0;

	//후에 SceneComponent로 옮김
	FTransform Transform;
};

class UBall : public UPrimitive
{
public:
	UBall(FTransform _Transform, FVector _Velocity) : UPrimitive(_Transform), Velocity(_Velocity)
	{
		//구형이므로 x로 반지름 판단
		SetRadius(Transform.Scale.x);
	}

	virtual ~UBall()
	{
	}

	FVector Velocity;

	virtual void Move(float deltaTime) override;
	void SetRadius(float newRadius);
};
