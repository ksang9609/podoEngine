#pragma once

#include "Object.h"
#include "ActorComponent.h"

class AActor : public UObject
{
	REFLECT_CLASS(AActor, UObject)
public:
	AActor();
	virtual ~AActor();

	void AddComponent(UActorComponent* actorComponent);
	bool RemoveComponent(uint32 componentUUID);

	void Update();
	void Render();

private:
	int32 getComponentIndex(uint32 componentUUID) const;

private:
	TArray<UActorComponent*> mComponents;
};

