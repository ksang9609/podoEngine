#include "MeshComponent.h"

#include <memory>

#include "Rendering/Mesh/StaticMesh.h"

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_OBJECT(UStaticMeshComponent, UMeshComponent)


private:
	std::shared_ptr<UStaticMesh> mStaticMesh;
};
