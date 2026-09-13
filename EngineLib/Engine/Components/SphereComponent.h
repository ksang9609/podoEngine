#pragma once

#include "PrimitiveComponent.h"

class USphereComponent : public UPrimitiveComponent
{
	DECLARE_OBJECT(USphereComponent, UPrimitiveComponent)
public:
	USphereComponent();
	virtual ~USphereComponent();

	//void Initialize(GraphicsManager* graphicsManager);
	//void Initialize(GraphicsManager* graphicsManager, FVector location, FRotator rotation, FVector scale3D);

	void Initialize();
	void Initialize(FVector location, FRotator rotation, FVector scale3D);
};
