#include "StaticMesh.h"

IMPLEMENT_CLASS_WITH_PROPERTIES(UStaticMesh, UObject);
IMPLEMENT_SERIALIZATION(UStaticMesh, UObject,
	{
		// TODO: Get handle from assest manager?
		mStaticMeshAssetRef = nullptr;
	}
)
