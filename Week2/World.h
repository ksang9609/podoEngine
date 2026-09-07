#pragma once

#include "Object.h"
#include "Actor.h"

#include "RenderInfo.h"
//struct FRenderInfo;

class UWorld final : public UObject
{
	REFLECT_CLASS(UWorld, UObject)
public:
	UWorld() = default;
	~UWorld();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	void AddActor(AActor* actor);
	bool RemoveActor(uint32 componentUUID);

	const TArray<FRenderInfo> GetRenderInfos();

	void Update();
	//void Render();
	void ClearRenderInfos();

private:
	int32 getActorIndex(uint32 actorUUID) const;

private:
	enum
	{
		DEFAULT_RESERVE_MEM = 1024U
	};
	
	// Todo: Must reserve
	TArray<AActor*> mActors;
	TArray<FRenderInfo> mRenderInfos;
};
