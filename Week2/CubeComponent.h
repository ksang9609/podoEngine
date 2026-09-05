#pragma once

#include "PrimitiveComponent.h"

class UCubeComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(UCubeComponent, UPrimitiveComponent)
public:
	UCubeComponent();
	UCubeComponent(GraphicsManager* graphicsManager);

	virtual ~UCubeComponent();

};
