// EngineLib/Rendering/Mesh/StaticMesh.h

#pragma once

#include <memory>

#include "Core/Object/Object.h"
#include "Core/Core.h"
#include "Core/Name.h"
#include "Rendering/VertexType.h"
#include "Material.h"

class UMaterial;

struct FStaticMeshSection
{
	FString Name;

	// 이 섹션이 사용할 FStaticMesh::MaterialSlots의 인덱스
   // -1이면 연결된 Material 슬롯이 없음
	int32 MaterialSlotIndex = -1; 

	int32 StartIndex = 0; // 해당 머티리얼이 적용되는 인덱스 버퍼 시작 위치
	int32 IndexCount = 0; // 해당 머티리얼이 적용되는 인덱스 버퍼 개수
	int32 GroupIndex = -1; // 해당 머티리얼이 속한 그룹 인덱스 (Obj 파일에서의 그룹)
};

// Coocked Data
struct FStaticMesh
{
	FName PathFileName;

	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	// 매시가 제공하는 기본 Material 슬롯
	//TArray<FMaterialSlot> MaterialSlots;

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

	void Initialize(std::unique_ptr<FStaticMesh> inStaticMesh, TArray<FMaterialSlot>&& inDefaultMaterials)
	{
		mStaticMeshAsset = std::move(inStaticMesh);
		mDefaultMaterials = std::move(inDefaultMaterials);
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

	const FMaterialSlot* GetMaterialSlot(int32 slotIndex) const
	{
		if (slotIndex < 0 || slotIndex >= mDefaultMaterials.Num())
		{
			return nullptr;
		}
		return &mDefaultMaterials[slotIndex];
	}

	const TArray<FMaterialSlot>& GetMaterialSlots() const
	{
		return mDefaultMaterials;
	}

private:
	std::unique_ptr<FStaticMesh> mStaticMeshAsset;

	TArray<FMaterialSlot> mDefaultMaterials;
};

