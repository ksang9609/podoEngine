#include "MeshComponent.h"

#include "Rendering/Mesh/StaticMesh.h"

struct FMaterialOverride
{
	bool bIsSet = false;
	FMaterial Material;
};

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_OBJECT(UStaticMeshComponent, UMeshComponent)
	DECLARE_SERIALIZATION()

	void Initialize(FVector location, FRotator rotation, FVector scale3D,
		FName textureName,
		const UStaticMesh* staticMeshOrNull = nullptr, bool bUseTexture = false);

	void SetStaticMesh(const UStaticMesh& staticMeshRef);

	const FStaticMesh* GetStaticMeshAsset() const;

	const FMaterial* GetMaterial(int32 slotIndex) const;
	bool SetMaterial(int32 slotIndex, const FMaterial& material);
	bool ClearMaterialOverride(int32 slotIndex);


protected:
	virtual FRenderInfo makeRenderInfo() const override;

private:
	void resetMaterialOverrides();

private:
	const UStaticMesh* mStaticMeshRef;

	// 인덱스는 FStaticMesh::MaterialSlots 인덱스와 동일하다.
	TArray<FMaterialOverride> mMaterialOverrides;

	FName mTextureName;
};
