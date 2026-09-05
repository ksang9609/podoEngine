#include "SphereComponent.h"

USphereComponent::USphereComponent()
{
}

void USphereComponent::Initialize(GraphicsManager* graphicsManager)
{
	UPrimitiveComponent::Initialize(graphicsManager, EPrimitive::EP_Sphere);
}

USphereComponent::~USphereComponent()
{

}
