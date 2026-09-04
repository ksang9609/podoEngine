#pragma once
#include "Vector.h"
#include "Rotator.h"

struct FTransform
{
	FTransform(){ }
	FTransform(FVector _Location, FRotator _Rotation, FVector _Scale) : Location(_Location), Rotation(_Rotation), Scale(_Scale)
	{

	}
	FVector Location = FVector(0);
	FRotator Rotation = FRotator(0, 0, 0);
	FVector Scale = FVector(1);

	FMatrix MakeMatrix() const
	{
		return  FMatrix::Scale(Scale) * Rotation.ToMatrix() * FMatrix::Translation(Location);
	}
};
