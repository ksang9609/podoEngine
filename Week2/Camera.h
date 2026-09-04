#pragma once
#include "Transform.h"

class FCamera
{
public:
	FCamera(FTransform _FTransform) : Transform(_FTransform) {}
	FTransform Transform;
};
