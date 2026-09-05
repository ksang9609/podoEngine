#include "PrimitiveComponent.h"

UPrimitiveComponent::UPrimitiveComponent()
{
}

void UPrimitiveComponent::Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive)
{
	mGraphicsManager = graphicsManager;
	mePrimitive = ePrimitive;
}

UPrimitiveComponent::~UPrimitiveComponent()
{
}

void UPrimitiveComponent::Render()
{
	// Todo: Fix renderer
	mGraphicsManager->Render(GetTransformMatrix(), mePrimitive);
}


