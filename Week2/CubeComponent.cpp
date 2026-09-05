
#include "CubeComponent.h"

UCubeComponent::UCubeComponent()
{
}

void UCubeComponent::Initialize(GraphicsManager* graphicsManager)
{
	UPrimitiveComponent::Initialize(graphicsManager, EPrimitive::EP_Cube);
}

UCubeComponent::~UCubeComponent()
{
}
