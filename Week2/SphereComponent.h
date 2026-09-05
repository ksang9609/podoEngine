#pragma once

#include "PrimitiveComponent.h"

class USphereComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(USphereComponent, UPrimitiveComponent)
public:
	USphereComponent();
	void Initialize(GraphicsManager* graphicsManager);
	virtual ~USphereComponent();
};
