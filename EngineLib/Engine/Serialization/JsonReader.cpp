// EngineLib/Engine/Serialization/JsonReader.cpp

#include "JSonReader.h"

FJsonReader::FJsonReader(std::string_view filePath)
	: FStructuredArchive(EArchiveMode::Loading)
{
	FFileManager fileManager;

	FString jsonString = fileManager.ReadFileToString(filePath);
	mRootJson = json::parse(jsonString);
}

bool FJsonReader::BeginObject(const char* name)
{
	if (HasError())
	{
		return false;
	}
	const json* currentJson = mJsonStack.IsEmpty() ? &mRootJson : *mJsonStack.rbegin();
	if (!currentJson->contains(name))
	{
		SetError();
		return false;
	}
	const json& childJson = (*currentJson)[name];
	if (!childJson.is_object())
	{
		SetError();
		return false;
	}
	mJsonStack.Add(&childJson);
	return true;
}

bool FJsonReader::Field(TNamedValue<bool> value)
{
	if (HasError())
	{
		return false;
	}
	const json* currentJson = mJsonStack.IsEmpty() ? &mRootJson : *mJsonStack.rbegin();
	if (!currentJson->contains(value.Name))
	{
		SetError();
		return false;
	}
	value.Value = (*currentJson)[value.Name].get<bool>();
	return true;
}

bool FJsonReader::Field(TNamedValue<int32> value)
{
	if (HasError())
	{
		return false;
	}
	const json* currentJson = mJsonStack.IsEmpty() ? &mRootJson : *mJsonStack.rbegin();
	if (!currentJson->contains(value.Name))
	{
		SetError();
		return false;
	}
	value.Value = (*currentJson)[value.Name].get<int32>();
	return true;
}

bool FJsonReader::Field(TNamedValue<uint32> value)
{
	if (HasError())
	{
		return false;
	}
	const json* currentJson = mJsonStack.IsEmpty() ? &mRootJson : *mJsonStack.rbegin();
	if (!currentJson->contains(value.Name))
	{
		SetError();
		return false;
	}
	value.Value = (*currentJson)[value.Name].get<uint32>();
	return true;
}

bool FJsonReader::Field(TNamedValue<float> value)
{
	if (HasError())
	{
		return false;
	}
	const json* currentJson = mJsonStack.IsEmpty() ? &mRootJson : *mJsonStack.rbegin();
	if (!currentJson->contains(value.Name))
	{
		SetError();
		return false;
	}
	value.Value = (*currentJson)[value.Name].get<float>();
	return true;
}

bool FJsonReader::Field(TNamedValue<double> value)
{
	if (HasError())
	{
		return false;
	}
	const json* currentJson = mJsonStack.IsEmpty() ? &mRootJson : *mJsonStack.rbegin();
	if (!currentJson->contains(value.Name))
	{
		SetError();
		return false;
	}
	value.Value = (*currentJson)[value.Name].get<double>();
	return true;
}

bool FJsonReader::Field(TNamedValue<FString> value)
{
	if (HasError())
	{
		return false;
	}
	const json* currentJson = mJsonStack.IsEmpty() ? &mRootJson : *mJsonStack.rbegin();
	if (!currentJson->contains(value.Name))
	{
		SetError();
		return false;
	}
	value.Value = (*currentJson)[value.Name].get<std::string>();
	return true;
}

bool FJsonReader::EndObject()
{
	if (mJsonStack.IsEmpty())
	{
		SetError();
		return false;
	}
	mJsonStack.Pop();
	return true;
}
