// EngineLib/Core/AssetManager.cpp

#include "AssetManager.h"

#include <memory>

#include "Core/BuiltinAssets.h"
#include "Core/Object/ObjectFactory.h"
#include "Rendering/Primitives/Cube.h"
#include "Rendering/Primitives/Sphere.h"

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
	const std::unique_ptr<UStaticMesh>* foundAsset = mStaticMeshAssets.Find(assetName);
	if (foundAsset)
	{
		return **foundAsset;
	}

	if (createStaticMeshAsset(assetName))
	{
		foundAsset = mStaticMeshAssets.Find(assetName);
		if (foundAsset)
		{
			return **foundAsset;
		}
	}

	throw std::runtime_error("Failed to find or create static mesh asset: ");
}

void FAssetManager::createBuiltinStaticMeshAssets()
{
	/* Cube */
	std::unique_ptr<FStaticMesh> cubeMeshData = std::make_unique<FStaticMesh>(CubeMesh);

	std::unique_ptr<UStaticMesh> cubeMeshAsset = std::unique_ptr<UStaticMesh>(
		FObjectFactory::ConstructObject<UStaticMesh>(std::move(cubeMeshData))
	);

	mStaticMeshAssets.Add(BuiltinAssets::Cube, std::move(cubeMeshAsset));

	/* Sphere */
	std::unique_ptr<FStaticMesh> sphereMeshData = std::make_unique<FStaticMesh>(SphereMesh);

	std::unique_ptr<UStaticMesh> sphereMeshAsset = std::unique_ptr<UStaticMesh>(
		FObjectFactory::ConstructObject<UStaticMesh>(std::move(sphereMeshData))
	);

	mStaticMeshAssets.Add(BuiltinAssets::Sphere, std::move(sphereMeshAsset));
}

bool FAssetManager::createStaticMeshAsset(const FName& assetName)
{
	assert(false && "createStaticMeshAsset is not implemented yet.");

	// TODO: Load static mesh data from Obj importer by assetName
	return false;
}

TArray<FName> FAssetManager::GetAllStaticMeshAssetKeys() const
{
	return mStaticMeshAssets.GetKeys();
}
