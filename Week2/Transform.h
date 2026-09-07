#pragma once
#include "Vector.h"
#include "Rotator.h"
#include "Matrix.h"
#include <cassert>

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

	FMatrix InverseMatrix() const
	{
		assert(Scale.x == 0.f || Scale.y == 0.f || Scale.z == 0.f);

		return {
			FMatrix::Translation(FVector(-Location.x, -Location.y, -Location.z))
			* FMatrix::Rotate(Rotation).Transpose()
			* FMatrix::Scale(FVector(1.0f / Scale.x, 1.0f / Scale.y, 1.0f / Scale.z))
		};
	}

	
};
