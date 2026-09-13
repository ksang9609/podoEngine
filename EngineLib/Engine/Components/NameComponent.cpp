#include "NameComponent.h"

IMPLEMENT_CLASS(UNameComponent, UBillboardComponent)

void UNameComponent::Initialize(const FString& nameText, FVector worldPositionOffset)
{
	UBillboardComponent::Initialize(worldPositionOffset, FRotator(), FVector(0));

	mNameText = nameText;

}

void UNameComponent::updateComponentToWorld(const FMatrix& parentTransform)
{
	// NameComponent always located over the actor's world position,
	// so we reuse mRelativeLocation as a world position offset from the actor's world position.

	FVector parentTranslation = parentTransform.GetTranslation();
	FVector worldPosition = parentTranslation + mRelativeLocation;
	mComponentToWorld = FTransform(worldPosition, FQuat::Identity(), mRelativeScale3D).MakeMatrix();
}

FRenderInfo UNameComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UBillboardComponent::makeRenderInfo();
	ERenderFlags renderFlags = renderInfo.eRenderFlags;

	// Remove primitive flags and add billboardtext flags
	renderFlags = renderFlags
		& ~ERenderFlags::RF_Primitive
		& ~ERenderFlags::RF_BoundingBox
		| ERenderFlags::RF_BillboardText;

	renderInfo.eRenderFlags = renderFlags;

	return renderInfo;
}
