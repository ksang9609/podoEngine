#pragma once

#include "Core/Object/Object.h"
#include "Core/Core.h"
#include "Core/Name.h"
#include "Rendering/VertexType.h"

// Coocked Data
struct FStaticMesh
{
	FName PathFileName;

	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	// ... need more?
};

class UStaticMesh : public UObject
{
	DECLARE_OBJECT(UStaticMesh, UObject)
	DECLARE_SERIALIZATION()
public:
	void Initialize() {};
	void Initialize(const FStaticMesh* inStaticMesh)
	{
		SetStaticMeshAsset(inStaticMesh);
	}

	const FName& GetAssetPathFileName() const
	{
		return mStaticMeshAssetRef->PathFileName;
	}

	void SetStaticMeshAsset(const FStaticMesh* inStaticMesh)
	{
		mStaticMeshAssetRef = inStaticMesh;
	}

	const FStaticMesh* GetStaticMeshAsset() const
	{
		return mStaticMeshAssetRef;
	}

private:
	const FStaticMesh* mStaticMeshAssetRef;
};

