#include "StaticMeshComponent.h"

static FBoundingBox calculateBounds(const TArray<FNormalVertex> vertices);

IMPLEMENT_CLASS_WITH_PROPERTIES(UStaticMeshComponent, UMeshComponent);
IMPLEMENT_SERIALIZATION(UStaticMeshComponent, UMeshComponent,
	{
		// TODO: Get static mesh reference from asset manager?
		mStaticMeshRef = nullptr;
	}
)

void UStaticMeshComponent::Initialize(FVector location, FRotator rotation, FVector scale3D,
	const UStaticMesh* staticMeshOrNull, bool bUseTexture)
{
	UPrimitiveComponent::Initialize(EPrimitive::EP_StaticMesh, location, rotation, scale3D, bUseTexture);
	mStaticMeshRef = staticMeshOrNull;

	if (mStaticMeshRef)
	{
		mLocalBounds = calculateBounds(mStaticMeshRef->GetStaticMeshAsset()->Vertices);
	}
	else
	{
		mLocalBounds = FBoundingBox{};
	}
}

void UStaticMeshComponent::SetStaticMesh(const UStaticMesh& staticMeshRef)
{
	mStaticMeshRef = &staticMeshRef;
	mLocalBounds = calculateBounds(mStaticMeshRef->GetStaticMeshAsset()->Vertices);
}

FRenderInfo UStaticMeshComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UMeshComponent::makeRenderInfo();

	renderInfo.StaticMesh = mStaticMeshRef ? mStaticMeshRef->GetStaticMeshAsset() : nullptr;

	return renderInfo;
}

static FBoundingBox calculateBounds(
	const TArray<FNormalVertex> vertices)
{
	FBoundingBox result{};
	result.min = vertices[0].pos;
	result.max = result.min;

	for (const auto& vertex : vertices)
	{
		const FVector position = vertex.pos;

		result.min.x = min(result.min.x, position.x);
		result.min.y = min(result.min.y, position.y);
		result.min.z = min(result.min.z, position.z);

		result.max.x = max(result.max.x, position.x);
		result.max.y = max(result.max.y, position.y);
		result.max.z = max(result.max.z, position.z);
	}

	return result;
}
