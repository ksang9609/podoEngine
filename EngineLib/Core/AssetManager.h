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
	const UStaticMesh* FindStaticMeshAssetOrNull(const FName& assetName) const;

	void CreateStaticMeshAsset(const FName& assetName, const FStaticMesh* staticMeshData);

	TArray<FName> GetAllStaticMeshAssetKeys() const;

	/* Resources */
	const FStaticMesh* FindStaticMeshDataOrNull(const FName& assetName) const;

	void CreateStaticMeshData(const FName& assetName, const FStaticMesh* staticMeshData);

	TArray<FName> GetAllStaticMeshDataKeys() const;

	//테스트 및 임시용.
	const UStaticMesh* LoadObjMesh(const FString& fileName);

private:
	/* Assets */
	TMap<FName, std::unique_ptr<UStaticMesh>> mStaticMeshAssets;

	/* Resources */
	TMap<FName, std::unique_ptr<FStaticMesh>> mStaticMeshData;

	void createPrimitiveStaticMeshAssets();
};
