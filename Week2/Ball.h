#pragma once
#include "Vector.h"

class UPrimitive
{
public:
	virtual ~UPrimitive() {}

	virtual void Move(float deltaTime) = 0;
	virtual bool Compact(const UPrimitive* others) const = 0;
	inline static int TotalNumBalls = 0;
};

class UBall : public UPrimitive
{
public:
	UBall(FVector _Location, FVector _Velocity, float _Radius) : Location(_Location), Velocity(_Velocity)
	{
		SetRadius(_Radius);
		++TotalNumBalls;
	}

	virtual ~UBall()
	{
		--TotalNumBalls;
	}

	//복사도 카운터 증가
	UBall(const UBall& o) noexcept : Location(o.Location), Velocity(o.Velocity), Radius(o.Radius), Mass(o.Mass)
	{
		++TotalNumBalls;
	}

	//이동은 증가 안함
	UBall(UBall&& o) noexcept : UBall(o) {}

	FVector Location;
	FVector Velocity;
	float Radius;
	float Mass;

	virtual void Move(float deltaTime) override;
	virtual bool Compact(const UPrimitive* Others) const override;
	void SetRadius(float newRadius);
};
