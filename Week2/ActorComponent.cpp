
#include "ActorComponent.h"

UActorComponent::UActorComponent()
{
}

UActorComponent::~UActorComponent()
{
}

void UActorComponent::Update(TArray<FRenderInfo>* outRenderInfos)
{
	// Todo: Do nothing, must override, some components may not call Update()
	// assert(false);
}

void UActorComponent::GetRenderInfos(TArray<FRenderInfo>* outRenderInfos)
{
	// Todo: Do nothing, must override, some components may not call GetRenderInfos()
	// assert(false);
}
