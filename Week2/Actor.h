#pragma once

#include "Object.h"
#include "ActorComponent.h"

struct FRenderInfo;

class AActor : public UObject
{
	REFLECT_CLASS(AActor, UObject)
public:
	AActor() = default;
	virtual ~AActor();

	void Initialize();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	void AddComponent(UActorComponent* actorComponent);
	bool RemoveComponent(uint32 componentUUID);

	void Update(TArray<FRenderInfo>* outRenderInfos);
	//void Render();

	//Actor가 클릭 되었을 때 호출
	void Click();
	void UnClick();
	bool IsClicked();

private:
	int32 getComponentIndex(uint32 componentUUID) const;

private:
	TArray<UActorComponent*> mComponents;
	bool mbClicked = false;
};

