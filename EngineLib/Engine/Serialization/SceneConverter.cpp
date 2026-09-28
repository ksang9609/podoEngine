#include "SceneConverter.h"

#include "ThirdParty/nlohmann/json.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <system_error>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace
{
	using Json = nlohmann::json;
	constexpr uint64_t MaxUUID = std::numeric_limits<uint32_t>::max();

	Json Name(const std::string& name)
	{
		return {{"BaseName", name}, {"Number", 0}};
	}

	void CheckArray(const Json& value, size_t size)
	{
		if (!value.is_array() || value.size() != size)
			throw std::runtime_error("Invalid numeric array size");
		for (const auto& number : value)
			if (!number.is_number() || !std::isfinite(number.get<double>()) ||
				std::abs(number.get<double>()) > std::numeric_limits<float>::max())
				throw std::runtime_error("Invalid numeric array value");
	}

	Json Vector(const Json& value)
	{
		CheckArray(value, 3);
		return {{"x", value[0]}, {"y", value[1]}, {"z", value[2]}};
	}

	Json Rotation(const Json& value)
	{
		CheckArray(value, 3);
		constexpr double Degrees = 180.0 / 3.14159265358979323846;
		Json result = {{"Roll", value[0].get<double>() * Degrees},
			{"Pitch", value[1].get<double>() * Degrees}, {"Yaw", value[2].get<double>() * Degrees}};
		for (const auto& angle : result)
			if (std::abs(angle.get<double>()) > std::numeric_limits<float>::max())
				throw std::runtime_error("Rotation exceeds float range");
		return result;
	}

	Json Convert(const Json& source)
	{
		const auto& next = source.at("NextUUID");
		if (!next.is_number_integer() || next.get<double>() < 1 || next.get<double>() > MaxUUID)
			throw std::runtime_error("NextUUID must be a positive uint32");
		uint64_t nextUUID = next.get<uint64_t>();
		const auto& primitives = source.at("Primitives");
		if (!primitives.is_object())
			throw std::runtime_error("Primitives must be an object");
		if (primitives.size() > MaxUUID - nextUUID || primitives.size() > INT_MAX)
			throw std::runtime_error("UUID or actor count overflow");

		std::map<uint32_t, const Json*> sorted;
		for (auto it = primitives.begin(); it != primitives.end(); ++it)
		{
			uint32_t uuid = 0;
			const auto& key = it.key();
			const auto parsed = std::from_chars(key.data(), key.data() + key.size(), uuid);
			if (parsed.ec != std::errc{} || parsed.ptr != key.data() + key.size() || uuid == 0)
				throw std::runtime_error("Invalid actor UUID: " + key);
			if (uuid >= nextUUID)
				throw std::runtime_error("Actor UUID must be below NextUUID: " + key);
			if (!sorted.emplace(uuid, &it.value()).second)
				throw std::runtime_error("Duplicate numeric actor UUID: " + key);
		}

		Json actors = {{"Count", sorted.size()}};
		size_t index = 0;
		for (const auto& [uuid, primitive] : sorted)
		{
			if (primitive->at("Type") != "StaticMeshComp")
				throw std::runtime_error("Unsupported primitive Type at UUID " + std::to_string(uuid));
			const auto asset = primitive->at("ObjStaticMeshAsset").get<std::string>();
			Json component = {
				{"ClassName", "UStaticMeshComponent"}, {"Name", Name("StaticMeshComponent")},
				{"UUID", nextUUID}, {"ParentUUID", -1},
				{"RelativeLocation", Vector(primitive->at("Location"))},
				{"RelativeRotation", Rotation(primitive->at("Rotation"))},
				{"RelativeScale3D", Vector(primitive->at("Scale"))},
				{"mStaticMeshAssetKey", Name(asset)}, {"mePrimitive", 6},
				{"mbUseTexture", true}, {"mbShowBoundingBox", true},
				{"mColor", {{"R", 1.0}, {"G", 1.0}, {"B", 1.0}, {"A", 1.0}}},
				{"mMaterialAssetKeys", {{"Count", 0}}}};
			actors[std::to_string(index++)] = {
				{"ClassName", "AActor"}, {"Name", Name("StaticMeshActor")}, {"UUID", uuid},
				{"mRootComponentUUID", nextUUID++},
				{"mComponents", {{"0", std::move(component)}, {"Count", 1}}}};
		}
		const auto& camera = source.at("PerspectiveCamera");
		for (const auto* key : {"FOV", "NearClip", "FarClip"})
			CheckArray(camera.at(key), 1);
		return {{"NextUUID", nextUUID},
			{"PerspectiveCamera", {{"FovDegree", camera.at("FOV")[0]},
				{"NearZ", camera.at("NearClip")[0]}, {"FarZ", camera.at("FarClip")[0]},
				{"Location", Vector(camera.at("Location"))}, {"Rotation", Rotation(camera.at("Rotation"))}}},
			{"World", {{"ClassName", "UWorld"}, {"UUID", 0}, {"Name", Name("UWorld")},
				{"mActors", std::move(actors)}}}};
	}
}

bool FSceneConverter::ConvertFile(const std::filesystem::path& inputPath,
	const std::filesystem::path& outputPath, std::string& outError)
{
	outError.clear();
	std::filesystem::path temporary;
	try
	{
		const auto input = std::filesystem::weakly_canonical(std::filesystem::absolute(inputPath));
		const auto output = std::filesystem::weakly_canonical(std::filesystem::absolute(outputPath));
		if (input == output || (std::filesystem::exists(output) && std::filesystem::equivalent(input, output)))
			throw std::runtime_error("Input and output paths must differ");
		std::ifstream stream(input, std::ios::binary);
		if (!stream) throw std::runtime_error("Cannot open input scene");
		// Reject duplicate JSON keys before the DOM parser can overwrite them.
		std::vector<std::set<std::string>> objectKeys;
		auto validateKeys = [&objectKeys](int, Json::parse_event_t event, Json& value)
		{
			if (event == Json::parse_event_t::object_start) objectKeys.emplace_back();
			else if (event == Json::parse_event_t::object_end) objectKeys.pop_back();
			else if (event == Json::parse_event_t::key &&
				!objectKeys.back().insert(value.get<std::string>()).second)
				throw std::runtime_error("Duplicate JSON key: " + value.get<std::string>());
			return true;
		};
		const Json converted = Convert(Json::parse(stream, validateKeys));
		stream.close();

		// Reserve a unique sibling file; Windows replacement preserves the old output on failure.
		wchar_t tempName[MAX_PATH];
		if (!GetTempFileNameW(output.parent_path().c_str(), L"scn", 0, tempName))
			throw std::system_error(GetLastError(), std::system_category(), "Cannot create temporary scene");
		temporary = tempName;
		std::ofstream destination(temporary, std::ios::binary | std::ios::trunc);
		destination.exceptions(std::ios::failbit | std::ios::badbit);
		destination << converted.dump(4) << '\n';
		destination.close();
		if (!MoveFileExW(temporary.c_str(), output.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			throw std::system_error(GetLastError(), std::system_category(), "Cannot replace output scene");
		return true;
	}
	catch (const std::exception& error)
	{
		outError = error.what();
		if (!temporary.empty())
		{
			std::error_code ignored;
			std::filesystem::remove(temporary, ignored);
		}
		return false;
	}
}
