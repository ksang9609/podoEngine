#include "PrimitiveComponent.h"

UPrimitiveComponent::UPrimitiveComponent()
{
}

void UPrimitiveComponent::Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D)
{
	USceneComponent::Initialize(location, rotation, scale3D);

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


