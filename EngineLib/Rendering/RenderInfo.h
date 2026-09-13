#pragma once

#include "Core/Math/Transform.h"
#include "Core/Object/Object.h"
#include "Core/Math/FBoundingBox.h"

struct FRenderInfo
{
	EPrimitive ePrimitive;
	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;
	ERenderFlags eRenderFlags;

	FBoundingBox LocalBounds{};
	FBoundingBox WorldBounds{};

	// Return world matrix for billboard quads to face the camera
	// Get FRotator input because current camera rotation is stored in FRotator.
	// If camera stores rotation in FQuat, we can use FQuat to calculate billboard matrix.
	FMatrix GetBillboardTransformMatrix(const FRotator& cameraRotation) const
	{
		if (ePrimitive != EPrimitive::EP_BillboardQuad)
		{
			return WorldTransformMatrix;
		}
		const FMatrix& world = WorldTransformMatrix;
		const FVector location = FVector(world.M[3][0], world.M[3][1], world.M[3][2]);
		//const FVector scale = {
		//	world.GetUnitAxis(EAxis::X).Length(),
		//	world.GetUnitAxis(EAxis::Y).Length(),
		//	world.GetUnitAxis(EAxis::Z).Length(),
		//};
		const FVector scale = FVector(1); // Billboard quad should not be scaled by world matrix, keep it uniform scale
		return FMatrix::Scale(scale) * FMatrix::Rotate(cameraRotation) * FMatrix::Translation(location);
	}

	bool bUseTexture = false;
};
