#include "PrimitiveComponent.h"

UPrimitiveComponent::UPrimitiveComponent()
{
}

UPrimitiveComponent::UPrimitiveComponent(GraphicsManager* graphicsManager, EPrimitive ePrimitive)
	: mGraphicsManager(graphicsManager)
	, mePrimitive(ePrimitive)
{
}

UPrimitiveComponent::~UPrimitiveComponent()
{
}

void UPrimitiveComponent::Render()
{
	// Todo: Fix renderer
	mGraphicsManager->Render(GetTransformMatrix(), mePrimitive);
}


