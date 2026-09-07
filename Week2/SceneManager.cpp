#include "SceneManager.h"

#include <format>

#include "EngineStatics.h"
#include "FileManager.h"
#include "JsonUtil.h"
#include "ObjectFactory.h"
#include "PrimitiveComponent.h"
#include "TArray.h"
#include "World.h"

void FSceneManager::NewScene()
{
	if (mCurrentWorld == nullptr)
	{
		delete mCurrentWorld;
	}

	mCurrentWorld = FObjectFactory::ConstructObject<UWorld>();
}

void FSceneManager::SaveScene(
	std::string_view sceneName,
	const FFileManager& fileManager)
{
	FString fileName = kSceneDataDir;
	fileName += FString("/");
	fileName += sceneName;
	fileName += kSceneDataSuffix;

	// Read the current scene data to read the Version
	uint32 version = 0;

	try
	{
		FString readSceneString = fileManager.ReadFileToString(fileName);
		json::JSON readSceneJson = json::JSON::Load(readSceneString);

		if (!readSceneJson.hasKey("Version") || readSceneJson.at("Version").JSONType() != json::JSON::Class::Integral)
		{
			version = 0;
		}
		else
		{
			version = readSceneJson.at("Version").ToInt();
		}
	}
	catch (const std::exception& e)
	{
		// If the file does not exist or cannot be read, we can assume it's a new scene and set version to 0
		version = 0;
	}

	json::JSON writeSceneJson = json::JSON::Make(json::JSON::Class::Object);
	json::JSON worldJson = json::JSON::Make(json::JSON::Class::Object);
	mCurrentWorld->SerializeClass(worldJson);

	writeSceneJson["Version"] = version;
	writeSceneJson["NextUUID"] = UEngineStatics::GetNextUUID();
	writeSceneJson["World"] = worldJson;

	FString jsonString = FString(writeSceneJson.dump(1, "  "));
	fileManager.WriteStringToFile(fileName, jsonString);
}

void FSceneManager::LoadScene(
	std::string_view sceneName,
	const FFileManager& fileManager)
{
	FString fileName = kSceneDataDir;
	fileName += FString("/");
	fileName += sceneName;
	fileName += kSceneDataSuffix;

	FString jsonString = fileManager.ReadFileToString(fileName);

	json::JSON readSceneJson = json::JSON::Load(jsonString);

	if (!readSceneJson.hasKey("NextUUID") || readSceneJson.at("NextUUID").JSONType() != json::JSON::Class::Integral)
	{
		throw std::runtime_error(std::format("Scene file {} does not contain a valid NextUUID field.", fileName));
	}
	uint32 nextUUID = readSceneJson.at("NextUUID").ToInt();
	json::JSON worldJson = readSceneJson.at("World");

	UWorld* newWorld = FObjectFactory::LoadObject<UWorld>(worldJson);
	if (!newWorld)
	{
		throw std::runtime_error(std::format("Failed to load world from scene: {}", sceneName));
	}
	UEngineStatics::SetNextUUID(nextUUID);

	// Replace the contents of mCurrentWorld with newWorld
	delete mCurrentWorld;
	mCurrentWorld = newWorld;
}

void FSceneManager::Update()
{
	if (mCurrentWorld)
	{
		mCurrentWorld->Update();
	}
}

const TArray<FRenderInfo> FSceneManager::GetRenderInfos()
{
	if (mCurrentWorld)
	{
		return mCurrentWorld->GetRenderInfos();
	}
	return TArray<FRenderInfo>();
}

//
//FSceneData FSceneManager::ReadSceneData(
//	std::string_view sceneName,
//	const FFileManager& fileManager)
//{
//	FString fileName = sceneName;
//	fileName += kSceneDataSuffix;
//
//	json::JSON jsonData = json::JSON::Load(fileManager.ReadFileToString(fileName));
//	FSceneData sceneData = FSceneData(jsonData);
//	return sceneData;
//}
//
//UWorld* FSceneManager::BuildWorldFromSceneData(const FSceneData& sceneData)
//{
//	//UWorld* newWorld = FObjectFactory::ConstructObject<UWorld>();
//
//	//for (const auto& [UUID, primitiveData] : sceneData.Primitives) 
//	//{
//	//	// TODO: Replace AActor creation logic later
//	//	AActor* newActor = FObjectFactory::ConstructObject<AActor>();
//	//	UPrimitiveComponent* newPrimitiveComponent =
//	//		FObjectFactory::ConstructObject<UPrimitiveComponent>(
//	//			);
//	//}
//
//	//UEngineStatics::SetNextUUID(sceneData.NextUUID);
//	throw std::logic_error("BuildWorldFromSceneData is not implemented yet.");
//	return nullptr;
//}
