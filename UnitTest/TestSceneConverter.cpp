#ifndef SCENE_CONVERTER_STANDALONE
#include "pch.h"
#endif
#include "Editor/SceneConverter.h"
#include "Engine/Serialization/JsonReader.h"
#include "ThirdParty/nlohmann/json.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <source_location>

namespace
{
	using Json = nlohmann::json;
	void Require(bool condition, std::source_location location = std::source_location::current())
	{
		if (!condition) throw std::runtime_error("Scene converter verification failed at line " + std::to_string(location.line()));
	}
	Json Read(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		return Json::parse(file);
	}
	void Write(const std::filesystem::path& path, const Json& value)
	{
		std::ofstream file(path);
		file << value.dump();
	}
	void VerifyReader(const std::filesystem::path& path, uint32 expectedNext, int32 expectedCount)
	{
		FJsonReader reader(path.string());
		uint32 next = 0;
		reader.Field(TNamedValue{"NextUUID", next});
		Require(next == expectedNext && reader.BeginObject("World"));
		Require(reader.BeginObject("mActors"));
		int32 count = 0;
		reader.Field(TNamedValue{"Count", count});
		Require(count == expectedCount && !reader.HasError());
		reader.EndObject();
		reader.EndObject();
		Require(reader.BeginObject("PerspectiveCamera"));
		float fov = 0;
		reader.Field(TNamedValue{"FovDegree", fov});
		Require(!reader.HasError());
	}
	void VerifyConverter()
	{
		const auto directory = std::filesystem::current_path() /
			("scene-converter-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
		std::filesystem::create_directory(directory);
		struct Cleanup
		{
			std::filesystem::path Path;
			~Cleanup() { std::error_code error; std::filesystem::remove_all(Path, error); }
		} cleanup{directory};
		const auto input = directory / "legacy.scene";
		const auto output = directory / "converted.Scene";
		Json primitive = {{"Type", "StaticMeshComp"}, {"ObjStaticMeshAsset", "Data/apple_mid.obj"},
			{"Location", {1, 2, 3}}, {"Scale", {2, 3, 4}},
			{"Rotation", {0.1, 0.2, 0.3}}};
		Json source = {{"NextUUID", 20}, {"Primitives", {{"10", primitive}, {"2", primitive}}},
			{"PerspectiveCamera", {{"FOV", {60}}, {"NearClip", {0.1}}, {"FarClip", {100}},
				{"Location", {4, 5, 6}}, {"Rotation", {0.1, 0.2, 0.3}}}}};
		std::string error;
		Write(input, source);
		if (!FSceneConverter::ConvertFile(input, output, error)) throw std::runtime_error(error);
		Require(error.empty());
		const auto result = Read(output);
		VerifyReader(output, 22, 2);
		Require(result.at("NextUUID") == 22);
		const auto& actors = result.at("World").at("mActors");
		Require(actors.at("Count") == 2 && actors.at("0").at("UUID") == 2 && actors.at("1").at("UUID") == 10);
		for (int i = 0; i < 2; ++i)
		{
			const auto& actor = actors.at(std::to_string(i));
			const auto& mesh = actor.at("mComponents").at("0");
			Require(actor.at("mRootComponentUUID") == 20 + i && mesh.at("UUID") == 20 + i);
			Require(actor.at("mComponents").at("Count") == 1 && mesh.at("ParentUUID") == -1);
			Require(mesh.at("mbUseTexture") == true && mesh.at("mePrimitive") == 6);
			Require(mesh.at("mStaticMeshAssetKey").at("BaseName") == "Data/apple_mid.obj");
			Require(mesh.at("RelativeLocation").at("y") == 2 && mesh.at("RelativeScale3D").at("z") == 4);
			const auto& rotation = mesh.at("RelativeRotation");
			Require(std::abs(rotation.at("Roll").get<double>() - 5.7295779513) < 1e-8);
			Require(std::abs(rotation.at("Pitch").get<double>() - 11.4591559026) < 1e-8);
			Require(std::abs(rotation.at("Yaw").get<double>() - 17.1887338539) < 1e-8);
			Require(rotation == result.at("PerspectiveCamera").at("Rotation"));
		}
		Require(result.at("PerspectiveCamera").at("FovDegree") == 60);
		auto reject = [&](const Json& invalid)
		{
			Write(input, invalid);
			Require(!FSceneConverter::ConvertFile(input, output, error) && !error.empty());
			Require(Read(output) == result);
		};
		Json invalid = source;
		invalid["Primitives"]["0"] = primitive; reject(invalid);
		invalid = source; invalid["Primitives"]["02"] = primitive; reject(invalid);
		invalid = source; invalid["NextUUID"] = 10; reject(invalid);
		invalid = source; invalid["NextUUID"] = 4294967295ULL; reject(invalid);
		invalid = source; invalid["NextUUID"] = -1; reject(invalid);
		invalid = source; invalid["Primitives"]["2"]["Type"] = "Cube"; reject(invalid);
		invalid = source; invalid["Primitives"]["2"]["Location"] = {1, 2}; reject(invalid);
		invalid = source; invalid["PerspectiveCamera"].erase("FOV"); reject(invalid);
		invalid = source; invalid["Primitives"]["4294967296"] = primitive; reject(invalid);
		{
			std::ofstream malformed(input);
			malformed << "{\"NextUUID\":20,\"NextUUID\":21}";
		}
		Require(!FSceneConverter::ConvertFile(input, output, error) && Read(output) == result);
		{
			std::ofstream malformed(input);
			malformed << "{invalid";
		}
		Require(!FSceneConverter::ConvertFile(input, output, error) && Read(output) == result);
		Write(input, source);
		Require(!FSceneConverter::ConvertFile(input, input, error) && Read(input) == source);
		Require(!FSceneConverter::ConvertFile(directory / "absent.scene", output, error));
		Require(!FSceneConverter::ConvertFile(input, directory / "absent" / "out.Scene", error));
		Require(!FSceneConverter::ConvertFile(input, directory, error));
		Require(Read(output) == result);
		source["NextUUID"] = 100;
		source["Primitives"] = Json::object();
		Write(directory / "empty.scene", source);
		Require(FSceneConverter::ConvertFile(directory / "empty.scene", output, error) && error.empty());
		Require(Read(output).at("NextUUID") == 100 && Read(output).at("World").at("mActors").at("Count") == 0);
	}
}

#ifdef SCENE_CONVERTER_STANDALONE
// Standalone runner also exercises the production converter on a supplied legacy file.
int main(int argc, char** argv)
{
	try
	{
		VerifyConverter();
		std::cout << "Scene converter tests passed\n";
		if (argc == 3)
		{
			std::string error;
			if (!FSceneConverter::ConvertFile(argv[1], argv[2], error))
				throw std::runtime_error(error);
			const auto source = Read(argv[1]);
			const auto result = Read(argv[2]);
			const auto& actors = result.at("World").at("mActors");
			const auto count = source.at("Primitives").size();
			VerifyReader(argv[2], source.at("NextUUID").get<uint32>() + static_cast<uint32>(count), static_cast<int32>(count));
			uint32 previous = 0;
			for (size_t i = 0; i < count; ++i)
			{
				const auto& actor = actors.at(std::to_string(i));
				const auto uuid = actor.at("UUID").get<uint32>();
				Require(uuid > previous);
				previous = uuid;
				const auto& mesh = actor.at("mComponents").at("0");
				Require(mesh.at("UUID") == source.at("NextUUID").get<uint32>() + i);
				Require(actor.at("mRootComponentUUID") == mesh.at("UUID"));
				Require(mesh.at("mbUseTexture") == true);
				Require(mesh.at("mStaticMeshAssetKey").at("BaseName") ==
					source.at("Primitives").at(std::to_string(uuid)).at("ObjStaticMeshAsset"));
			}
			std::cout << "Converted scene saved\n";
		}
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}
#else
TEST(SceneConverter, ConvertsAndPreservesFilesOnFailure)
{
	EXPECT_NO_THROW(VerifyConverter());
}
#endif
