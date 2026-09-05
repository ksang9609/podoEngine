#include "Actor.h"

AActor::AActor()
{
}

AActor::~AActor()
{
	for (UActorComponent* removeComponent : mComponents)
	{
		delete removeComponent;
	}
}

void AActor::AddComponent(UActorComponent* actorComponent)
{
	assert(actorComponent);
	assert(getComponentIndex(actorComponent->UUID) == -1);

	mComponents.Add(actorComponent);
}

bool AActor::RemoveComponent(uint32 componentUUID)
{
	int32 componentIndex = getComponentIndex(componentUUID);
	if (componentIndex == -1)
	{
		return false;
	}

	mComponents.RemoveAt(componentIndex, 1);

	return true;
}

void AActor::Render()
{
	for (UActorComponent* component : mComponents)
	{
		component->Render();
	}
}

int32 AActor::getComponentIndex(uint32 componentUUID) const
{
	for (uint32 i = 0; i < mComponents.Num(); ++i)
	{
		if (mComponents[i]->UUID == componentUUID)
		{
			return i;
		}
	}

	return -1;
}
