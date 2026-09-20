// EngineLib/Core/AssetManager.h

#pragma once

#include <memory>

#include "Core/Container/TArray.h"
#include "Core/Container/TMap.h"
#include "Core/Name.h"

#include "Rendering/Mesh/StaticMesh.h"


// In charge of managing assets and **resources**
// TODO: separate resource manager from asset manager
// Assets: UStaticMesh, etc
// Resources: FStaticMesh, etc
class FAssetManager
{
public:
	FAssetManager();

	/* Assets */
	// TODO: Replace or null fuction with FindStaticMeshAssetOrAdd
	const UStaticMesh* FindStaticMeshAssetOrNull(const FName& assetName) const;
	const UStaticMesh& FindStaticMeshAssetOrAdd(const FName& assetName);

	// TODO: Remove this function
	void CreateStaticMeshAsset(const FName& assetName, FStaticMesh* staticMeshData);

	TArray<FName> GetAllStaticMeshAssetKeys() const;

private:
	/* Assets */
	TMap<FName, std::unique_ptr<UStaticMesh>> mStaticMeshAssets;

	void createBuiltinStaticMeshAssets();
	bool createStaticMeshAsset(const FName& assetName);
};
