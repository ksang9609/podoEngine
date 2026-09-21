#include "MeshComponent.h"

#include "Rendering/Mesh/StaticMesh.h"

struct FMaterialOverride
{
	bool bIsSet = false;
	const UMaterial* OverridedMaterialRef = nullptr;
};

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_OBJECT(UStaticMeshComponent, UMeshComponent)
	DECLARE_SERIALIZATION()

	void Initialize(
		FVector location,
		FRotator rotation,
		FVector scale3D,
		FName textureName,
		const UStaticMesh* staticMeshOrNull,
		bool bUseTexture = false
	);

	static std::span<const FPropertyInfo> GetDeclaredProperties();

	void SetStaticMesh(const UStaticMesh& staticMeshRef);
	const FName& GetStaticMeshAssetKey() const { return mStaticMeshRef->GetAssetPathFileName(); }
	const FName& GetMaterialAssetKey(int32 slotIndex) const;
	uint32 GetMaterialSlotCount() const { return mStaticMeshRef ? mStaticMeshRef->GetDefaultMaterials().Num() : 0; }

	const UStaticMesh* GetStaticMeshAsset() const;

	const UMaterial* GetMaterialAsset(int32 slotIndex) const;
	bool SetMaterial(int32 slotIndex, const UMaterial& materialAsset);
	bool ClearMaterialOverride(int32 slotIndex);


protected:
	virtual FRenderInfo makeRenderInfo() const override;

private:
	void resetMaterialOverrides();

private:
	const UStaticMesh* mStaticMeshRef = nullptr;
	FName mStaticMeshAssetKey;

	// 인덱스는 FStaticMesh::MaterialSlots 인덱스와 동일하다.
	TArray<FMaterialOverride> mMaterialOverrides;

};
