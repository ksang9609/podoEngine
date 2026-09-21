#include "JsonArchive.h"

#include <memory>
#include <stdexcept>

#include "Core/Object/ObjectFactory.h"
#include "Engine/EngineStatics.h"
#include "Engine/World.h"

namespace
{
	constexpr uint32 SceneFormatVersion = 0;
}

json::JSON FJsonArchive::SerializeWorld(const UWorld& world)
{
	json::JSON worldJson = json::JSON::Make(json::JSON::Class::Object);
	world.SerializeClass(worldJson);

	json::JSON archiveJson = json::JSON::Make(json::JSON::Class::Object);

	archiveJson["Version"] = SceneFormatVersion;
	archiveJson["NextUUID"] = UEngineStatics::GetNextUUID();
	archiveJson["World"] = std::move(worldJson);

	return archiveJson;
}

UWorld* FJsonArchive::DeserializeWorld(const json::JSON& inJson)
{
	if (inJson.JSONType() != json::JSON::Class::Object)
	{
		throw std::runtime_error("Scene archive requires a JSON object");
	}

	if (!inJson.hasKey("Version") ||
		inJson.at("Version").JSONType() != json::JSON::Class::Integral)
	{
		throw std::runtime_error("Scene archive requires an integral Version");
	}

	const long version = inJson.at("Version").ToInt();
	if (version != SceneFormatVersion)
	{
		throw std::runtime_error("Unsupported scene archive version");
	}

	if (!inJson.hasKey("NextUUID") ||
		inJson.at("NextUUID").JSONType() != json::JSON::Class::Integral)
	{
		throw std::runtime_error("Scene archive requires an integral NextUUID");
	}

	const long nextUUID = inJson.at("NextUUID").ToInt();
	if (nextUUID < 0)
	{
		throw std::runtime_error("Scene archive NextUUID cannot be negative");
	}

	if (!inJson.hasKey("World") ||
		inJson.at("World").JSONType() != json::JSON::Class::Object)
	{
		throw std::runtime_error("Scene archive requires a World object");
	}

	const json::JSON& worldJson = inJson.at("World");
	if (!worldJson.hasKey("ClassName")
		|| worldJson.at("ClassName").JSONType() != json::JSON::Class::String
		|| FString(worldJson.at("ClassName").ToString()) != UWorld::GetClass()->Name)
	{
		throw std::runtime_error("Scene archive World must have class UWorld");
	}

	std::unique_ptr<UWorld> newWorld(FObjectFactory::ConstructUnInitializedObject<UWorld>());
	if (!newWorld)
	{
		throw std::runtime_error("Failed to create UWorld");
	}

	newWorld->DeserializeClass(worldJson);
	UEngineStatics::SetNextUUID(static_cast<uint32>(nextUUID));

	return newWorld.release();
}
