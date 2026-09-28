#pragma once

#include <filesystem>
#include <string>

// Converts the legacy Primitives format without creating engine objects.
class FSceneConverter
{
public:
	static bool ConvertFile(const std::filesystem::path& inputPath,
		const std::filesystem::path& outputPath, std::string& outError);
};
