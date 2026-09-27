//EngineLib/Core/Archive/StructuredArchive.h

#pragma once

#include "Core/Archive/Archive.h"

template<typename T>
struct TNamedValue
{
	const char* Name;
	T& Value;
};

class FStructuredArchive
{
public:
	virtual ~FStructuredArchive() = default;

	virtual bool BeginObject(const char* name) = 0;
	virtual bool Field(TNamedValue<bool> value) = 0;
	virtual bool Field(TNamedValue<int32> value) = 0;
	virtual bool Field(TNamedValue<uint32> value) = 0;
	virtual bool Field(TNamedValue<float> value) = 0;
	virtual bool Field(TNamedValue<double> value) = 0;
	virtual bool Field(TNamedValue<FString> value) = 0;
	virtual bool EndObject() = 0;

	bool IsLoading() const { return mMode == EArchiveMode::Loading; }
	bool IsSaving() const { return mMode == EArchiveMode::Saving; }
	bool HasError() const { return bHasError; }
	void SetError() { bHasError = true; }

protected:
	explicit FStructuredArchive(EArchiveMode mode) : mMode(mode) {}

private:
	EArchiveMode mMode;
	bool bHasError = false;
};


// For basic types that have a Serialize method in the archive
template<typename T>
	requires requires(FStructuredArchive& archive, TNamedValue<T> item) { archive.Field(item); }
FStructuredArchive& operator<<(FStructuredArchive& archive, TNamedValue<T> namedValue)
{
	if (archive.HasError())
	{
		return archive;
	}
	archive.Field(namedValue);
	return archive;
}

// For custom types that implement a Serialize method
template<typename T>
	requires requires(T& value, FStructuredArchive& archive) { value.Serialize(archive); }
FStructuredArchive& operator<<(FStructuredArchive& archive, TNamedValue<T> namedValue)
{
	if (archive.HasError())
	{
		return archive;
	}

	if (!archive.BeginObject(namedValue.Name))
	{
		return archive;
	}

	namedValue.Value.Serialize(archive);
	archive.EndObject();

	return archive;
}
