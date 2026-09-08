#pragma once

#include "Object.h"
#include "ActorComponent.h"

class UWorld;
struct FRenderInfo;
struct FTransform;
class USceneComponent;

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
	void AddRootSceneComponent(USceneComponent* sceneComponent);
	bool RemoveComponent(uint32 componentUUID);

	FTransform GetTransform() const;

	void Update(TArray<FRenderInfo>* outRenderInfos);
	//void Render();

	void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const;
	bool GetFirstRenderInfo(FRenderInfo& outRenderInfo) const;

	void SetLocation(FVector location);
	void SetRotation(FRotator rotation);
	void SetScale(FVector scale);

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
	
	USceneComponent* mRootComponent = nullptr;
	TArray<UActorComponent*> mComponents;
	bool mbPressed = false;
	bool mbStarted = false;
};

