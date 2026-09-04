#pragma once
#include "Vector.h"
#include "Rotator.h"

struct FTransform
{
	FTransform(){ }

	FVector Location = FVector(0);
	FRotator Rotation = FRotator(0, 0, 0);
	FVector Scale = FVector(1);

	FMatrix MakeMatrix() const
	{
		return Rotation.ToMatrix();
	}
};
