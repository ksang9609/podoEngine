#pragma once

#include "Object.h"
#include "ActorComponent.h"

struct FRenderInfo;

class AActor : public UObject
{
	REFLECT_CLASS(AActor, UObject)
public:
	AActor();
	virtual ~AActor();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	void AddComponent(UActorComponent* actorComponent);
	bool RemoveComponent(uint32 componentUUID);

	void Update();
	//void Render();

	void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos);

private:
	int32 getComponentIndex(uint32 componentUUID) const;

private:
	TArray<UActorComponent*> mComponents;
};

