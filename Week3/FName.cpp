#include "Core.h"
#include "FName.h"

#include <vector>
#include <algorithm>
#include <unordered_map>

namespace
{
	std::vector<std::string> GDisplayNamePool;
	std::vector<std::string> GComparisonNamePool;

	std::unordered_map<std::string, int32> GDisplayNamePoolLookup;
	std::unordered_map<std::string, int32> GComparisonNamePoolLookup;

	static std::string ToLower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
			{
				return static_cast<char>(std::tolower(c));
			}
		);

		return str;
	}

	static int32 FindOrAdd(std::vector<std::string>& Pool, std::unordered_map<std::string, int32>& Lookup, const std::string& Name)
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
	std::string DisplayName = pStr;
	std::string ComparisonName = ToLower(DisplayName);

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
	const std::string& A = GComparisonNamePool[ComparisonIndex];
	const std::string& B = GComparisonNamePool[Other.ComparisonIndex];

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
