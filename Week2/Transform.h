#pragma once
#include "Vector.h"
#include "Rotator.h"
#include "Matrix.h"

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
		return  FMatrix::Scale(Scale) * FMatrix::Rotate(Rotation) * FMatrix::Translation(Location);
	}
};
