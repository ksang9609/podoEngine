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
	resetMaterialOverrides();

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
	resetMaterialOverrides();
	mStaticMeshAssetKey = staticMeshRef.GetAssetPathFileName();

	mLocalBounds = calculateBounds(mStaticMeshRef->GetStaticMeshAsset()->Vertices);
}

const FStaticMesh* UStaticMeshComponent::GetStaticMeshAsset() const
{
	return mStaticMeshRef ? mStaticMeshRef->GetStaticMeshAsset() : nullptr;
}

const FMaterial* UStaticMeshComponent::GetMaterial(int32 slotIndex) const
{
	const FStaticMesh* mesh = GetStaticMeshAsset();

	if (!mesh || slotIndex < 0 || slotIndex >= mesh->MaterialSlots.Num())
	{
		return nullptr;
	}

	if (slotIndex < mMaterialOverrides.Num())
	{
		const FMaterialOverride& materialOverride = mMaterialOverrides[slotIndex];

		if (materialOverride.bIsSet)
		{
			return &materialOverride.Material;
		}
	}

	return &mesh->MaterialSlots[slotIndex].DefaultMaterial;
}

bool UStaticMeshComponent::SetMaterial(int32 slotIndex, const FMaterial& material)
{
	if (slotIndex < 0 || slotIndex >= mMaterialOverrides.Num())
	{
		return false;
	}

	FMaterialOverride& materialOverride = mMaterialOverrides[slotIndex];

	materialOverride.bIsSet = true;
	materialOverride.Material = material;

	return true;
}

bool UStaticMeshComponent::ClearMaterialOverride(int32 slotIndex)
{
	if (slotIndex < 0 || slotIndex >= mMaterialOverrides.Num())
	{
		return false;
	}

	mMaterialOverrides[slotIndex] = FMaterialOverride{};
	return true;
}

FRenderInfo UStaticMeshComponent::makeRenderInfo() const
{
	FRenderInfo renderInfo = UMeshComponent::makeRenderInfo();

	renderInfo.MeshName = mStaticMeshRef ? mStaticMeshRef->GetAssetPathFileName() : FName();
	renderInfo.TextureName = mTextureName;
	renderInfo.StaticMesh = mStaticMeshRef ? mStaticMeshRef->GetStaticMeshAsset() : nullptr;

	if (renderInfo.StaticMesh)
	{
		renderInfo.Materials.Reserve(renderInfo.StaticMesh->MaterialSlots.Num());

		for (int32 slotIndex = 0; slotIndex < renderInfo.StaticMesh->MaterialSlots.Num(); ++slotIndex)
		{
			const FMaterial* material = GetMaterial(slotIndex);

			if (material)
			{
				renderInfo.Materials.Add(*material);
			}
		}
	}

	return renderInfo;
}

void UStaticMeshComponent::resetMaterialOverrides()
{
	mMaterialOverrides.Reset(0);

	const FStaticMesh* mesh = GetStaticMeshAsset();
	if (!mesh)
	{
		return;
	}

	mMaterialOverrides.Reserve(mesh->MaterialSlots.Num());

	for (int32 i = 0; i < mesh->MaterialSlots.Num(); ++i)
	{
		mMaterialOverrides.Add(FMaterialOverride{});
	}
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
