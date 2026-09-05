#pragma once

#include "PrimitiveComponent.h"

class UCubeComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(UCubeComponent, UPrimitiveComponent)
public:
	UCubeComponent();

	UCubeComponent(GraphicsManager* graphicsManager, FVector location, FRotator rotation, FVector scale3D);
	UCubeComponent(GraphicsManager* graphicsManager);
	void Initialize(GraphicsManager* graphicsManager);

	virtual ~UCubeComponent();

};
