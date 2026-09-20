#include "StaticMeshComponent.h"

static FBoundingBox calculateBounds(const TArray<FNormalVertex> vertices);

IMPLEMENT_CLASS_WITH_PROPERTIES(UStaticMeshComponent, UMeshComponent);

void UStaticMeshComponent::SerializeClass(json::JSON& outJson) const
{
	UMeshComponent::SerializeClass(outJson);
	for (const FPropertyInfo& property : ClassInfo.DeclaredProperties)
	{
		property.Serialize(property, this, outJson["Properties"]);
	}
}

void UStaticMeshComponent::DeserializeClass(const json::JSON& inJson)
{
	UMeshComponent::DeserializeClass(inJson);
	const json::JSON& properties = inJson.at("Properties");

	for (const FPropertyInfo& property : ClassInfo.DeclaredProperties)
	{
		// Older scene files do not contain the static mesh asset key.
		if (!properties.hasKey(property.JsonKey))
		{
			continue;
		}

		property.Deserialize(property, this, properties);
	}
}

void UStaticMeshComponent::PostDeserialize()
{
	UMeshComponent::PostDeserialize();
	mStaticMeshRef = nullptr;
}

void UStaticMeshComponent::Initialize(
	FVector location,
	FRotator rotation,
	FVector scale3D,
	FName textureName,
	const UStaticMesh* staticMeshOrNull,
	bool bUseTexture
)
{
	UPrimitiveComponent::Initialize(EPrimitive::EP_StaticMesh, location, rotation, scale3D, bUseTexture);
	mStaticMeshRef = staticMeshOrNull;
	mStaticMeshAssetKey = staticMeshOrNull
		? staticMeshOrNull->GetAssetPathFileName()
		: FName();
	mTextureName = textureName;

	mLocalBounds = FBoundingBox{};

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
	mStaticMeshAssetKey = staticMeshRef.GetAssetPathFileName();

	mLocalBounds = calculateBounds(mStaticMeshRef->GetStaticMeshAsset()->Vertices);
}

FRenderInfo UStaticMeshComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UMeshComponent::makeRenderInfo();

	renderInfo.MeshName = mStaticMeshRef ? mStaticMeshRef->GetAssetPathFileName() : FName();
	renderInfo.TextureName = mTextureName;
	renderInfo.StaticMesh = mStaticMeshRef ? mStaticMeshRef->GetStaticMeshAsset() : nullptr;

	return renderInfo;
}

static FBoundingBox calculateBounds(const TArray<FNormalVertex> vertices)
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

std::span<const FPropertyInfo> UStaticMeshComponent::GetDeclaredProperties()
{
	static const FPropertyInfo Properties[] =
	{
		REFLECT_PROPERTY(
			UStaticMeshComponent,
			mStaticMeshAssetKey
		),

		REFLECT_PROPERTY(
			UStaticMeshComponent,
			mTextureName
		)
	};

	return Properties;
}
