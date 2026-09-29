#include "BillboardComponent.h"

#include "Rendering/RenderInfo.h"

IMPLEMENT_CLASS(UBillboardComponent, UPrimitiveComponent);

UBillboardComponent::UBillboardComponent()
{
}

void UBillboardComponent::Initialize(FVector location, FRotator rotation, FVector scale3D)
{
	UPrimitiveComponent::Initialize(EPrimitive::EP_BillboardQuad, location, rotation, scale3D);
}

void UBillboardComponent::updateRenderInfo()
{
	UPrimitiveComponent::updateRenderInfo();
	mRenderInfo.eRenderFlags = mRenderInfo.eRenderFlags | ERenderFlags::RF_Billboard;
}
