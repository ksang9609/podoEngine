// EngineLib/Engine/Serialization/JsonReader.h

#pragma once

#include "Core/Archive/StructuredArchive.h"

#include "ThirdParty/nlohmann/json.hpp"

#include "Core/IO/FileManager.h"
#include "Core/Container/TArray.h"

class FJsonReader final : public FStructuredArchive
{
public:
	using json = nlohmann::json;

	explicit FJsonReader(std::string_view fileDirPath, std::string_view fileName);

	virtual bool BeginObject(const char* name) override;
	virtual bool Field(TNamedValue<bool> value) override;
	virtual bool Field(TNamedValue<int32> value) override;
	virtual bool Field(TNamedValue<uint32> value) override;
	virtual bool Field(TNamedValue<float> value) override;
	virtual bool Field(TNamedValue<double> value) override;
	virtual bool Field(TNamedValue<FString> value) override;
	virtual bool EndObject() override;

private:
	json mRootJson;
	TArray<const json*> mJsonStack;
};
