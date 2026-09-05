#pragma once

#include "PrimitiveComponent.h"

class UCubeComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(UCubeComponent, UPrimitiveComponent)
public:
	UCubeComponent();
	void Initialize(GraphicsManager* graphicsManager);

	virtual ~UCubeComponent();

};
