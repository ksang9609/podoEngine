#pragma once

#include "SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
	REFLECT_CLASS(UPrimitiveComponent, USceneComponent)
public:
	UPrimitiveComponent();

	UPrimitiveComponent(GraphicsManager* graphicsManager, EPrimitive ePrimitive);
	virtual ~UPrimitiveComponent();

	virtual void Render();

protected:
	GraphicsManager* mGraphicsManager;
	EPrimitive mePrimitive;
};


