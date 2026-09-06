#include "SceneManager.h"

#include "Json/json.hpp"

#include "FileManager.h"
#include "World.h"
#include "EngineStatics.h"
#include "FObjectFactory.h"
#include "PrimitiveComponent.h"

void FSceneManager::NewScene(UWorld* world)
{

}

void FSceneManager::SaveScene(
	std::string_view sceneName,
	const FFileManager& fileManager,
	const UWorld* world)
{

}

void FSceneManager::LoadScene(
	std::string_view sceneName,
	const FFileManager& fileManager,
	UWorld* world)
{

}

FSceneData FSceneManager::ReadSceneData(
	std::string_view sceneName,
	const FFileManager& fileManager)
{
	FString fileName = sceneName;
	fileName += kSceneDataSuffix;

	json::JSON jsonData = json::JSON::Load(fileManager.ReadFileToString(fileName));
	FSceneData sceneData = FSceneData(jsonData);
	return sceneData;
}

UWorld* FSceneManager::BuildWorldFromSceneData(const FSceneData& sceneData)
{
	//UWorld* newWorld = FObjectFactory::ConstructObject<UWorld>();

	//for (const auto& [UUID, primitiveData] : sceneData.Primitives)
	//{
	//	// TODO: Replace AActor creation logic later
	//	AActor* newActor = FObjectFactory::ConstructObject<AActor>();
	//	UPrimitiveComponent* newPrimitiveComponent =
	//		FObjectFactory::ConstructObject<UPrimitiveComponent>(
	//			);
	//}

	//UEngineStatics::SetNextUUID(sceneData.NextUUID);
	throw std::logic_error("BuildWorldFromSceneData is not implemented yet.");
	return nullptr;
}
