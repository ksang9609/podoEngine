#pragma once

#include "Core/Object/Object.h"
#include "Core/Core.h"
#include "Rendering/VertexType.h"

// Coocked Data
struct FStaticMesh
{
	FString PathFileName;

	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	// ... need more?
};

class UStaticMesh : public UObject
{
	DECLARE_OBJECT(UStaticMesh, UObject)
public:

	const FString& GetAssetPathFileName() const
	{
		return StaticMeshAsset->PathFileName;
	}

	void SetStaticMeshAsset(FStaticMesh* inStaticMesh)
	{
		StaticMeshAsset = inStaticMesh;
	}

private:
	FStaticMesh* StaticMeshAsset;
};

