#pragma once

#include "Object.h"
#include "Actor.h"

class UWorld : public UObject
{
public:
	UWorld();
	~UWorld();

	void AddActor(AActor* actor);
	bool RemoveActor(uint32 componentUUID);

	void Update();
	void Render();

private:
	int32 getActorIndex(uint32 actorUUID) const;

private:
	TArray<AActor*> mActors;
};
