// EngineLib/Core/AssetManager.cpp

#include "AssetManager.h"

#include <memory>

#include "Core/BuiltinAssets.h"
#include "Core/Object/ObjectFactory.h"
#include "Rendering/Primitives/Cube.h"
#include "Rendering/Primitives/Sphere.h"
#include <filesystem>
#include <Editor/Console.h>
#include <Rendering/Mesh/ObjImporter.h>
#include "Rendering/Mesh/Material.h"
#include "Rendering/Mesh/StaticMesh.h"

FAssetManager::FAssetManager()
{
	createBuiltinStaticMeshAssets();
}

const UStaticMesh* FAssetManager::FindStaticMeshAssetOrNull(const FName& assetName) const
{
	const std::unique_ptr<UStaticMesh>* foundAsset = mStaticMeshAssets.Find(assetName);
	if (foundAsset)
	{
		return foundAsset->get();
	}
	return nullptr;
}

const UStaticMesh& FAssetManager::FindStaticMeshAssetOrAdd(const FName& assetName)
{
	// Cube, Sphere 같은 내장 에셋도 여기서 찾는다.
	if (const UStaticMesh* existing = FindStaticMeshAssetOrNull(assetName))
	{
		return *existing;
	}

	const FString fileName = assetName.ToString();

	// 상대 경로, "..", 경로 구분자 등을 정리한다.
	std::error_code error;
	const std::filesystem::path canonicalPath =std::filesystem::weakly_canonical( std::filesystem::path(fileName.CStr()), error);

	if (!error && fileName.Len() > 0)
	{
		const std::string pathString = canonicalPath.generic_string();
		const FName meshKey(pathString.c_str());

		// 같은 파일이 다른 경로 표기로 이미 등록됐는지 확인한다.
		if (const UStaticMesh* existing = FindStaticMeshAssetOrNull(meshKey))
		{
			return *existing;
		}

		if (createStaticMeshAsset(meshKey))
		{
			if (const UStaticMesh* created =FindStaticMeshAssetOrNull(meshKey))
			{
				return *created;
			}
		}
	}

	throw std::runtime_error( std::string("Failed to find or create static mesh asset: ") + fileName.CStr());
}

const UMaterial* FAssetManager::FindMaterialAssetOrNull(const FName& assetName) const
{
	const std::unique_ptr<UMaterial>* foundAsset = mMaterialAssets.Find(assetName);
	if (foundAsset)
	{
		return foundAsset->get();
	}
	return nullptr;
}

void FAssetManager::createBuiltinStaticMeshAssets()
{
	/* Cube */
	std::unique_ptr<FMaterial> cubeMaterialData = std::make_unique<FMaterial>(CubeMaterial);
	std::unique_ptr<UMaterial> cubeMaterialAsset(
		FObjectFactory::ConstructObject<UMaterial>(BuiltinAssets::CubeMaterial, std::move(cubeMaterialData))
	);
	TArray<const UMaterial*> cubeDefaultMaterials;
	cubeDefaultMaterials.Add(cubeMaterialAsset.get());

	std::unique_ptr<FStaticMesh> cubeMeshData = std::make_unique<FStaticMesh>(CubeMesh);

	std::unique_ptr<UStaticMesh> cubeMeshAsset = std::unique_ptr<UStaticMesh>(
		FObjectFactory::ConstructObject<UStaticMesh>(std::move(cubeMeshData), std::move(cubeDefaultMaterials))
	);

	mMaterialAssets.Add(BuiltinAssets::CubeMaterial, std::move(cubeMaterialAsset));
	mStaticMeshAssets.Add(BuiltinAssets::CubeMesh, std::move(cubeMeshAsset));

	/* Sphere */
	std::unique_ptr<FMaterial> sphereMaterialData = std::make_unique<FMaterial>(SphereMaterial);
	std::unique_ptr<UMaterial> sphereMaterialAsset(
		FObjectFactory::ConstructObject<UMaterial>(BuiltinAssets::SphereMaterial, std::move(sphereMaterialData))
	);
	TArray<const UMaterial*> sphereDefaultMaterials;
	sphereDefaultMaterials.Add(sphereMaterialAsset.get());

	std::unique_ptr<FStaticMesh> sphereMeshData = std::make_unique<FStaticMesh>(SphereMesh);

	std::unique_ptr<UStaticMesh> sphereMeshAsset = std::unique_ptr<UStaticMesh>(
		FObjectFactory::ConstructObject<UStaticMesh>(std::move(sphereMeshData), std::move(sphereDefaultMaterials))
	);

	mMaterialAssets.Add(BuiltinAssets::SphereMaterial, std::move(sphereMaterialAsset));
	mStaticMeshAssets.Add(BuiltinAssets::SphereMesh, std::move(sphereMeshAsset));
}

bool FAssetManager::createStaticMeshAsset(const FName& assetName)
{
	const FString fileName = assetName.ToString();

	FObjImportResult imported;

	if (!FObjImporter::ParseAndConvert(fileName, imported) ||
		!imported.meshData ||
		imported.materialSlots.IsEmpty())
	{
		return false;
	}

	// Create Material Assets
	TArray<const UMaterial*> defaultMaterialRefs;
	for (auto& materialSlot : imported.materialSlots)
	{
		const FName materialKey = FName(materialSlot.Name);
		if (!mMaterialAssets.Contains(materialKey))
		{
			std::unique_ptr<FMaterial> materialData = std::make_unique<FMaterial>(materialSlot.DefaultMaterial);
			std::unique_ptr<UMaterial> materialAsset = std::unique_ptr<UMaterial>(
				FObjectFactory::ConstructObject<UMaterial>(materialKey, std::move(materialData))
			);
			mMaterialAssets.Add(materialKey, std::move(materialAsset));
		}

		// 생성 여부와 관계없이, 원본 슬롯 순서대로 추가
		defaultMaterialRefs.Add(FindMaterialAssetOrNull(materialKey));
	}

	// Create Static Mesh Asset
	imported.meshData->PathFileName = assetName;

	std::unique_ptr<UStaticMesh> asset(
		FObjectFactory::ConstructObject<UStaticMesh>(std::move(imported.meshData), std::move(defaultMaterialRefs))
	);

	if (!asset)
	{
		return false;
	}

	mStaticMeshAssets[assetName] = std::move(asset);

	return true;
}

TArray<FName> FAssetManager::GetAllStaticMeshAssetKeys() const
{
	return mStaticMeshAssets.GetKeys();
}

TArray<FName> FAssetManager::GetAllMaterialAssetKeys() const
{
	return mMaterialAssets.GetKeys();
}
