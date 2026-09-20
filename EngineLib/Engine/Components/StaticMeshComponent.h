#include "MeshComponent.h"

#include "Rendering/Mesh/StaticMesh.h"

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
	const FName& GetStaticMeshAssetKey() const { return mStaticMeshAssetKey; }


protected:
	virtual FRenderInfo makeRenderInfo() const override;

private:
	const UStaticMesh* mStaticMeshRef = nullptr;
	FName mStaticMeshAssetKey;
	FName mTextureName;
};
