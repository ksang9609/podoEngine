#include "MeshComponent.h"

#include "Rendering/Mesh/StaticMesh.h"

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_OBJECT(UStaticMeshComponent, UMeshComponent)
	DECLARE_SERIALIZATION()

	void Initialize(FVector location, FRotator rotation, FVector scale3D,
		const UStaticMesh* staticMeshOrNull = nullptr, bool bUseTexture = false);

	void SetStaticMesh(const UStaticMesh& staticMeshRef);

protected:
	virtual FRenderInfo makeRenderInfo() const override;

private:
	const UStaticMesh* mStaticMeshRef = nullptr;
};
