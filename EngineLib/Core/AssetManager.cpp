// EngineLib/Core/AssetManager.cpp

#include "AssetManager.h"

#include <memory>

#include "Core/BuiltinAssets.h"
#include "Core/Object/ObjectFactory.h"
#include "Rendering/Primitives/Cube.h"
#include "Rendering/Primitives/Sphere.h"

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

	mStaticMeshData.Add(BuiltinAssets::CubeMesh, std::move(cubeMeshData));
	mStaticMeshAssets.Add(BuiltinAssets::CubeMesh, std::move(cubeMeshAsset));

	/* Sphere */
	std::unique_ptr<FStaticMesh> sphereMeshData = std::make_unique<FStaticMesh>(SphereMesh);

	std::unique_ptr<UStaticMesh> sphereMeshAsset = std::unique_ptr<UStaticMesh>(
		FObjectFactory::ConstructObject<UStaticMesh>(sphereMeshData.get())
	);

	mStaticMeshData.Add(BuiltinAssets::SphereMesh, std::move(sphereMeshData));
	mStaticMeshAssets.Add(BuiltinAssets::SphereMesh, std::move(sphereMeshAsset));
}

TArray<FName> FAssetManager::GetAllStaticMeshAssetPaths() const
{
	return mStaticMeshAssets.GetKeys();
}

TArray<FName> FAssetManager::GetAllStaticMeshDataPaths() const
{
	return mStaticMeshData.GetKeys();
}
