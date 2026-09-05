#include "SphereComponent.h"

USphereComponent::USphereComponent()
{
}

void USphereComponent::Initialize(GraphicsManager* graphicsManager)
{
	Initialize(graphicsManager, FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f));
}

void USphereComponent::Initialize(GraphicsManager* graphicsManager, FVector location, FRotator rotation, FVector scale3D)
{
	UPrimitiveComponent::Initialize(graphicsManager, EPrimitive::EP_Sphere, location, rotation, scale3D);
}

USphereComponent::~USphereComponent()
{
}
