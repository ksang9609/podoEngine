
#include "World.h"
#include "RenderInfo.h"

UWorld::~UWorld()
{
	for (AActor* removeActor : mActors)
	{
		delete removeActor;
	}
}

void UWorld::AddActor(AActor* actor)
{
	assert(actor != nullptr);
	assert(getActorIndex(actor->UUID) == -1);

	mActors.Add(actor);
}

bool UWorld::RemoveActor(uint32 componentUUID)
{
	int32 componentIndex = getActorIndex(componentUUID);
	if (componentIndex == -1)
	{
		return false;
	}

	mActors.RemoveAt(componentIndex, 1);

	return true;
}

const TArray<FRenderInfo> UWorld::GetRenderInfos()
{
	return mRenderInfos;
}

void UWorld::Update()
{
	mRenderInfos.Reset(DEFAULT_RESERVE_MEM);

	for (AActor* actor : mActors)
	{
		// Update actors

		//
		actor->GetRenderInfos(&mRenderInfos);
	}
}

/*
void UWorld::Render()
{
	for (AActor* actor : mActors)
	{
		actor->Render();
	}
}
*/

int32 UWorld::getActorIndex(uint32 actorUUID) const
{
	for (uint32 i = 0; i < mActors.Num(); ++i)
	{
		if (mActors[i]->UUID == actorUUID)
		{
			return i;
		}
	}

	return -1;
}
