#pragma once

#include "Transform.h"
#include "ObejctID.h"

struct FRenderInfo
{
	EPrimitive ePrimitive;
	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
};
