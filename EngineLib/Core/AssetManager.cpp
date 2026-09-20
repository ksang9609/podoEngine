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
#include <Rendering/Mesh/Material.h>

FAssetManager::FAssetManager()
{
	createPrimitiveStaticMeshAssets();
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

const FStaticMesh* FAssetManager::FindStaticMeshDataOrNull(const FName& assetName) const
{
	const std::unique_ptr<FStaticMesh>* foundData = mStaticMeshData.Find(assetName);
	if (foundData)
	{
		return foundData->get();
	}
	return nullptr;
}

void FAssetManager::createPrimitiveStaticMeshAssets()
{
	/* Cube */
	std::unique_ptr<FStaticMesh> cubeMeshData = std::make_unique<FStaticMesh>(CubeMesh);

	std::unique_ptr<UStaticMesh> cubeMeshAsset = std::unique_ptr<UStaticMesh>(
		FObjectFactory::ConstructObject<UStaticMesh>(cubeMeshData.get())
	);

	mStaticMeshData.Add(BuiltinAssets::Cube, std::move(cubeMeshData));
	mStaticMeshAssets.Add(BuiltinAssets::Cube, std::move(cubeMeshAsset));

	/* Sphere */
	std::unique_ptr<FStaticMesh> sphereMeshData = std::make_unique<FStaticMesh>(SphereMesh);

	std::unique_ptr<UStaticMesh> sphereMeshAsset = std::unique_ptr<UStaticMesh>(
		FObjectFactory::ConstructObject<UStaticMesh>(sphereMeshData.get())
	);

	mStaticMeshData.Add(BuiltinAssets::Sphere, std::move(sphereMeshData));
	mStaticMeshAssets.Add(BuiltinAssets::Sphere, std::move(sphereMeshAsset));
}

TArray<FName> FAssetManager::GetAllStaticMeshAssetKeys() const
{
	return mStaticMeshAssets.GetKeys();
}

TArray<FName> FAssetManager::GetAllStaticMeshDataKeys() const
{
	return mStaticMeshData.GetKeys();
}

const UStaticMesh* FAssetManager::LoadObjMesh(const FString& fileName)
{
	if (fileName.Len() == 0)
	{
		return nullptr;
	}

	// 상대 경로, "..", 경로 구분자 등을 정리한다.
	std::error_code error;
	const std::filesystem::path canonicalPath =std::filesystem::weakly_canonical(std::filesystem::path(fileName.CStr()), error);

	if (error)
	{
		return nullptr;
	}

	const std::string pathString = canonicalPath.generic_string();
	const FString resolvedPath(pathString.c_str());

	// 여기서 meshKey를 만든다.
	// AssetManager의 메시 캐시를 찾을 때 사용하는 키다.
	const FName meshKey(resolvedPath);

	// 이미 로드한 메시라면 재사용한다.
	if (const UStaticMesh* existing =
		FindStaticMeshAssetOrNull(meshKey))
	{
		return existing;
	}

	FObjImportResult imported;

	if (!FObjImporter::ParseAndConvert(resolvedPath, imported) ||
		!imported.meshData)
	{
		return nullptr;
	}

	imported.meshData->PathFileName = meshKey;

	std::unique_ptr<UStaticMesh> asset(FObjectFactory::ConstructObject<UStaticMesh>(imported.meshData.get()));

	if (!asset)
	{
		return nullptr;
	}

	const UStaticMesh* result = asset.get();

	// 같은 meshKey로 CPU 메시 데이터와 UStaticMesh를 보관한다.
	mStaticMeshData[meshKey] = std::move(imported.meshData);
	mStaticMeshAssets[meshKey] = std::move(asset);

	return result;
}
