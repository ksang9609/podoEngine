#include "NameComponent.h"

IMPLEMENT_CLASS(UNameComponent, UBillboardComponent)

void UNameComponent::Initialize(const FString& nameText, FVector worldPositionDelta)
{
	UBillboardComponent::Initialize(worldPositionDelta, FRotator(), FVector(0));

	mNameText = nameText;
	
}
