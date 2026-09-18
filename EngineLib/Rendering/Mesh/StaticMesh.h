#pragma once

#include "Core/Object/Object.h"
#include "Core/Core.h"
#include "Rendering/VertexType.h"
#include "ObjImporter.h"

struct FStaticMeshSection
{
	FString Name;
	int32 MaterialIndex;
	int32 StartIndex;
};

// Coocked Data
struct FStaticMesh
{
	FString PathFileName;

	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	TArray<FObjMaterialInfo> Materials;
	TArray<FStaticMeshSection> Sections;
};

class UStaticMesh : public UObject
{
	DECLARE_OBJECT(UStaticMesh, UObject)
	DECLARE_SERIALIZATION()
public:

	const FString& GetAssetPathFileName() const
	{
		return mStaticMeshAssetRef->PathFileName;
	}

	void SetStaticMeshAsset(FStaticMesh* inStaticMesh)
	{
		mStaticMeshAssetRef = inStaticMesh;
	}

private:
	FStaticMesh* mStaticMeshAssetRef;
};

