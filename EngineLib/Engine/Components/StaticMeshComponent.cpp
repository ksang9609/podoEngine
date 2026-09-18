#include "StaticMeshComponent.h"

IMPLEMENT_CLASS_WITH_PROPERTIES(UStaticMeshComponent, UMeshComponent);
IMPLEMENT_SERIALIZATION(UStaticMeshComponent, UMeshComponent,
	{
		// TODO: Get static mesh reference from asset manager?
		mStaticMeshRef = nullptr;
	}
)

void UStaticMeshComponent::Initialize(FVector location, FRotator rotation, FVector scale3D,
	UStaticMesh* staticMeshRef, bool bUseTexture)
{
	UPrimitiveComponent::Initialize(EPrimitive::EP_StaticMesh, location, rotation, scale3D, bUseTexture);
	mStaticMeshRef = staticMeshRef;
}

FRenderInfo UStaticMeshComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UMeshComponent::makeRenderInfo();
	return renderInfo;
}
