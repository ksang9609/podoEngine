#include "JsonArchive.h"
#include <stdexcept>

namespace
{
	constexpr uint32 SceneFormatVersion = 0;
}

json::JSON FJsonArchive::Serialize(const json::JSON& worldJson, uint32 nextUUID)
{
	json::JSON archiveJson = json::JSON::Make(json::JSON::Class::Object);

	archiveJson["Version"] = SceneFormatVersion;
	archiveJson["NextUUID"] = nextUUID;
	archiveJson["World"] = worldJson;

	return archiveJson;
}

FJsonArchiveData FJsonArchive::Deserialize(const json::JSON& archiveJson)
{
	if (archiveJson.JSONType() != json::JSON::Class::Object)
	{
		throw std::runtime_error("Archive requires a JSON object");
	}

	if (!archiveJson.hasKey("Version") || archiveJson.at("Version").JSONType() != json::JSON::Class::Integral)
	{
		throw std::runtime_error("Archive requires an integral Version");
	}

	if (archiveJson.at("Version").ToInt() != SceneFormatVersion)
	{
		throw std::runtime_error("Unsupported archive version");
	}

	if (!archiveJson.hasKey("NextUUID") || archiveJson.at("NextUUID").JSONType() != json::JSON::Class::Integral)
	{
		throw std::runtime_error("Archive requires an integral NextUUID");
	}

	const long nextUUID = archiveJson.at("NextUUID").ToInt();

	if (nextUUID < 0)
	{
		throw std::runtime_error("Archive NextUUID cannot be negative");
	}

	if (!archiveJson.hasKey("World") ||
		archiveJson.at("World").JSONType() != json::JSON::Class::Object)
	{
		throw std::runtime_error("Archive requires a World object");
	}

	return FJsonArchiveData{
		.NextUUID = static_cast<uint32>(nextUUID),
		.WorldJson = archiveJson.at("World")
	};
}

