#include "Core.h"
#include "Name.h"

#include <vector>
#include <algorithm>
#include <unordered_map>

namespace
{
	std::vector<FString> GDisplayNamePool;
	std::vector<FString> GComparisonNamePool;

	std::unordered_map<FString, int32> GDisplayNamePoolLookup;
	std::unordered_map<FString, int32> GComparisonNamePoolLookup;

	static FString ToLower(FString str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
			{
				return static_cast<char>(std::tolower(c));
			}
		);

		return str;
	}

	static int32 FindOrAdd(std::vector<FString>& Pool, std::unordered_map<FString, int32>& Lookup, const FString& Name)
	{
		auto It = Lookup.find(Name);

		if (It != Lookup.end())
		{
			return It->second;
		}

		const int32 NewIndex = static_cast<int32>(Pool.size());
		Pool.push_back(Name);
		Lookup.emplace(Name, NewIndex);

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
	if (DisplayIndex < 0 || DisplayIndex >= static_cast<int32>(GDisplayNamePool.size()))
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

	if (A < B)
	{
		return -1;
	}

	if (A > B)
	{
		return 1;
	}
	return 0;
}

