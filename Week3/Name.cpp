#include "Core.h"
#include "TArray.h"
#include "TMap.h"
#include "Name.h"

#include <vector>
#include <algorithm>
#include <unordered_map>

namespace
{
	TArray<FString> GDisplayNamePool;
	TArray<FString> GComparisonNamePool;

	TMap<FString, int32> GDisplayNamePoolLookup;
	TMap<FString, int32> GComparisonNamePoolLookup;

	static FString ToLower(FString str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
			{
				return static_cast<char>(std::tolower(c));
			}
		);

		return str;
	}

	static int32 FindOrAdd(TArray<FString>& Pool, TMap<FString, int32>& Lookup, const FString& Name)
	{
		int32* FoundIndex = Lookup.Find(Name);

		if (FoundIndex)
		{
			return *FoundIndex;
		}

		const int32 NewIndex = static_cast<int32>(Pool.Num());
		Pool.Add(Name);
		Lookup.Add(Name, NewIndex);

		return NewIndex;
	}
}

FName::FName()
	: DisplayIndex(-1),
	ComparisonIndex(-1)
{
}


FName::FName(const char* pStr)
{
	FString DisplayName = pStr;
	FString ComparisonName = ToLower(DisplayName);

	DisplayIndex = FindOrAdd(GDisplayNamePool, GDisplayNamePoolLookup, DisplayName);
	ComparisonIndex = FindOrAdd(GComparisonNamePool, GComparisonNamePoolLookup, ComparisonName);
}

FName::FName(const FString& str)
	: FName(str.CStr())
{
}

FString FName::ToString() const
{
	if (DisplayIndex < 0 || DisplayIndex >= static_cast<int32>(GDisplayNamePool.Num()))
	{
		return FString("None");
	}

	return FString(GDisplayNamePool[DisplayIndex]);
}


bool FName::operator==(const FName& Other) const
{
	return ComparisonIndex == Other.ComparisonIndex;
}

int32 FName::Compare(const FName& Other) const
{
	const FString& A = GComparisonNamePool[ComparisonIndex];
	const FString& B = GComparisonNamePool[Other.ComparisonIndex];

	int Result = std::strcmp(A.CStr(), B.CStr());

	if (Result < 0)
	{
		return -1;
	}

	if (Result > 0)
	{
		return 1;
	}

	return 0;
}

