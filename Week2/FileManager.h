#pragma once

#include <filesystem>

#include "Core.h"

inline constexpr std::string_view kDefaultRootPath = ".\\";
inline constexpr std::string_view kDefaultAssetsPath = ".\\Assets\\";
inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FFileManager
{
public:
	FFileManager();
	FFileManager(std::string_view fileDirPath);
	FFileManager(std::string_view fileDirPath, std::string_view rootPath);

	FString ReadFileToString(std::string_view fileName);
	void WriteStringToFile(std::string_view fileName, std::string_view content);

private:
	std::filesystem::path mFileDirPath;
	std::filesystem::path mRootPath;

	bool IsUnderRoot(const std::filesystem::path& filePath) const;
	bool IsUnderFileDir(const std::filesystem::path& filePath) const;
};

bool IsUnder(const std::filesystem::path& filePath, const std::filesystem::path& rootPath);
