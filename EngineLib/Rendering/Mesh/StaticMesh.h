#pragma once

#include <memory>

#include "Core/Object/Object.h"
#include "Core/Core.h"
#include "Core/Name.h"
#include "Rendering/VertexType.h"

class UMaterial;

struct FObjMaterialInfo
{
	FString Name;
	FVector AmbientColor = FVector(0.0f, 0.0f, 0.0f);   // Ka
	FVector DiffuseColor = FVector(1.f, 1.0f, 1.0f);   // Kd 
	FVector SpecularColor = FVector(0.0f, 0.0f, 0.0f);   // Ks	
	float   SpecularExponent = 0.0f;                      // Ns
	float   Alpha = 1.0f;
	FString DiffuseTexturePath; // map_Kd
	FString NormalTexturePath;  // map_bump or norm
	FString SpecularPath;
};

struct FStaticMeshSection
{
	FString Name; // 섹션 이름
	int32 MaterialIndex; // 해당 섹션에 적용되는 머티리얼 인덱스
	int32 StartIndex; // 해당 섹션의 인덱스 버퍼 시작 위치
	int32 IndexCount; // 해당 섹션의 인덱스 개수
	int32 GroupIndex = -1; // 해당 섹션이 속한 그룹 인덱스 (Obj 파일에서의 그룹)
};

// Coocked Data
struct FStaticMesh
{
	FName PathFileName;

	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	TArray<FObjMaterialInfo> Materials;
	TArray<FStaticMeshSection> Sections;
	TArray<FString> GroupNames;
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

