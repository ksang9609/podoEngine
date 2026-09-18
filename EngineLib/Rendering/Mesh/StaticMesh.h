#pragma once

#include "Core/Object/Object.h"
#include "Core/Core.h"
#include "Rendering/VertexType.h"
#include "ObjImporter.h"

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
	FString PathFileName;

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

