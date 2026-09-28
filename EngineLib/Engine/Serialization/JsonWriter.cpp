// EngineLib/Engine/Serialization/JsonWriter.h

#include "JsonWriter.h"

#include "ThirdParty/nlohmann/json.hpp"

using json = nlohmann::json;

FJsonWriter::FJsonWriter(std::string_view fileDirPath)
	: FStructuredArchive(EArchiveMode::Saving)
	, mFileManager(fileDirPath)
{
	// Initialize the JSON stack with an empty object
	mRootJson = json::object();
	mJsonStack.Add(&mRootJson);
}

bool FJsonWriter::BeginObject(const char* name)
{
	if (HasError())
	{
		return false;
	}

	if (mJsonStack.IsEmpty())
	{
		SetError();
		return false;
	}

	json& currentObject = *mJsonStack.Last();
	json newObject = json::object();

	currentObject[name] = newObject;
	mJsonStack.Add(&currentObject[name]);

	return true;
}

bool FJsonWriter::Field(TNamedValue<bool> value)
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}

	json& currentObject = *mJsonStack.Last();

	currentObject[value.Name] = value.Value;
	return true;
}

bool FJsonWriter::Field(TNamedValue<int32> value)
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}
	json& currentObject = *mJsonStack.Last();
	currentObject[value.Name] = value.Value;
	return true;
}

bool FJsonWriter::Field(TNamedValue<uint32> value)
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}
	json& currentObject = *mJsonStack.Last();
	currentObject[value.Name] = value.Value;
	return true;
}

bool FJsonWriter::Field(TNamedValue<float> value)
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}
	json& currentObject = *mJsonStack.Last();
	currentObject[value.Name] = value.Value;
	return true;
}

bool FJsonWriter::Field(TNamedValue<double> value)
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}
	json& currentObject = *mJsonStack.Last();
	currentObject[value.Name] = value.Value;
	return true;
}

bool FJsonWriter::Field(TNamedValue<FString> value)
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}
	json& currentObject = *mJsonStack.Last();
	currentObject[value.Name] = value.Value.CStr();
	return true;
}

bool FJsonWriter::EndObject()
{
	if (mJsonStack.Num() <= 1)
	{
		SetError();
		return false;
	}
	mJsonStack.Pop();
	return true;
}

void FJsonWriter::SaveToFile(std::string_view filePath)
{
	if (HasError() || mJsonStack.Num() != 1)
	{
		SetError();
		return;
	}

	FString jsonString = FString(mRootJson.dump(4, ' ')); // Pretty print with 4 spaces

	mFileManager.WriteStringToFile(filePath, jsonString);
}
