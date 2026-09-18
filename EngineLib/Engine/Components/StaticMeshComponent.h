#include "MeshComponent.h"

#include "Rendering/Mesh/StaticMesh.h"

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_OBJECT(UStaticMeshComponent, UMeshComponent)
	DECLARE_SERIALIZATION()

	void Initialize(FVector location, FRotator rotation, FVector scale3D,
		UStaticMesh* staticMeshRef, bool bUseTexture = false);

protected:
	virtual FRenderInfo makeRenderInfo() const override;

private:
	UStaticMesh* mStaticMeshRef;
};
