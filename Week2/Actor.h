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

	void Pressed();
	void UnPressed();
	void ClickStart();

	//매 프레임 시작 시 호출. 이번 프레임용 플래그를 지운다
	void BeginFrame();

	bool IsPressed() const;
	bool IsStarted() const;   // 이번 프레임에 눌리기 시작 (1프레임만)

private:
	int32 getComponentIndex(uint32 componentUUID) const;

private:
	TArray<UActorComponent*> mComponents;
	bool mbPressed = false;
	bool mbStarted = false;
};

