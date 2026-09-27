// EngineLib/Engine/Serialization/JsonWriter.h

#include "JsonWriter.h"

#include "ThirdParty/nlohmann/json.hpp"

using json = nlohmann::json;

FJsonWriter::FJsonWriter(std::string_view fileDirPath)
	: FStructuredArchive(EArchiveMode::Saving)
	, mFileManager(fileDirPath)
{
	// Initialize the JSON stack with an empty object
	mJsonStack.Add(json::object());
}

bool FJsonWriter::BeginObject(const char* name)
{
	json newObject = json::object();
	mJsonStack.Add(newObject);

	return true;
}

bool FJsonWriter::Field(TNamedValue<bool> value)
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}

	json& currentObject = mJsonStack[mJsonStack.Num() - 1];

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
	json& currentObject = mJsonStack[mJsonStack.Num() - 1];
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
	json& currentObject = mJsonStack[mJsonStack.Num() - 1];
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
	json& currentObject = mJsonStack[mJsonStack.Num() - 1];
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
	json& currentObject = mJsonStack[mJsonStack.Num() - 1];
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
	json& currentObject = mJsonStack[mJsonStack.Num() - 1];
	currentObject[value.Name] = value.Value.CStr();
	return true;
}

bool FJsonWriter::EndObject()
{
	if (mJsonStack.Num() == 0)
	{
		SetError();
		return false;
	}
	json completedObject = mJsonStack.Pop();
	if (mJsonStack.Num() > 0)
	{
		json& parentObject = mJsonStack[mJsonStack.Num() - 1];
		parentObject.update(completedObject);
	}
	else
	{
		SetError();
		return false;
	}
	return true;
}

void FJsonWriter::SaveToFile(const FString& filePath)
{
	if (mJsonStack.Num() != 1)
	{
		SetError();
		return;
	}
	json rootObject = mJsonStack.Pop();
	FString jsonString = FString(rootObject.dump(4, ' ')); // Pretty print with 4 spaces

	mFileManager.WriteStringToFile(filePath, jsonString);
}
