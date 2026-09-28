// EngineLib/Engine/Serialization/JsonWriter.h

#pragma once

#include <string_view>

#include "ThirdParty/nlohmann/json.hpp"

#include "Core/IO/FileManager.h"
#include "Core/Archive/StructuredArchive.h"
#include "Core/Container/TArray.h"

class FJsonWriter final : public FStructuredArchive
{
public:
	using json = nlohmann::json;

	explicit FJsonWriter(std::string_view fileDirPath);

	FJsonWriter(const FJsonWriter&) = delete;
	FJsonWriter& operator=(const FJsonWriter&) = delete;
	FJsonWriter(FJsonWriter&&) = delete;
	FJsonWriter& operator=(FJsonWriter&&) = delete;

	virtual bool BeginObject(const char* name) override;
	virtual bool Field(TNamedValue<bool> value) override;
	virtual bool Field(TNamedValue<int32> value) override;
	virtual bool Field(TNamedValue<uint32> value) override;
	virtual bool Field(TNamedValue<float> value) override;
	virtual bool Field(TNamedValue<double> value) override;
	virtual bool Field(TNamedValue<FString> value) override;
	virtual bool EndObject() override;

	void SaveToFile(std::string_view filePath);

private:
	FFileManager mFileManager;
	json mRootJson;
	TArray<json*> mJsonStack;
};
