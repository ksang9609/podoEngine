
#include "CubeComponent.h"

UCubeComponent::UCubeComponent()
{
}

UCubeComponent::UCubeComponent(GraphicsManager* graphicsManager)
	: UPrimitiveComponent(graphicsManager, EPrimitive::EP_Cube)
{
}

UCubeComponent::~UCubeComponent()
{
}
