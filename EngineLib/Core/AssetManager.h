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
	const UStaticMesh& FindStaticMeshAssetOrAdd(const FName& assetName);


	TArray<FName> GetAllStaticMeshAssetKeys() const;


	// 이미 로드된 UStaticMesh를 FName AssetPath로 찾는 함수
	const UStaticMesh* FindStaticMesh(const FName& assetPath) const;

private:
	/* Assets */
	TMap<FName, std::unique_ptr<UStaticMesh>> mStaticMeshAssets;

	void createBuiltinStaticMeshAssets();
	bool createStaticMeshAsset(const FName& assetName);
};
