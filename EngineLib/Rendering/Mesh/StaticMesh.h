#pragma once

#include <memory>

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
	void Initialize(FStaticMesh* inStaticMesh)
	{
		SetStaticMeshAsset(inStaticMesh);
	}

	void Initialize(std::unique_ptr<FStaticMesh> inStaticMesh)
	{
		SetStaticMeshAsset(std::move(inStaticMesh));
	}

	const FName& GetAssetPathFileName() const
	{
		return mStaticMeshAsset->PathFileName;
	}

	void SetStaticMeshAsset(FStaticMesh* inStaticMesh)
	{
		mStaticMeshAsset.reset(inStaticMesh);
	}

	void SetStaticMeshAsset(std::unique_ptr<FStaticMesh> inStaticMesh)
	{
		mStaticMeshAsset = std::move(inStaticMesh);
	}

	const FStaticMesh* GetStaticMeshAsset() const
	{
		return mStaticMeshAsset.get();
	}

private:
	std::unique_ptr<FStaticMesh> mStaticMeshAsset;
};

