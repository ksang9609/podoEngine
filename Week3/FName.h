#pragma once

#include "Core.h"

struct FName
{
	FName();
	FName(const char* pStr);
	FName(const FString& str);

	int32 Compare(const FName& other) const;
	bool operator==(const FName& other) const;

	int32 DisplayIndex;
	int32 ComparisonIndex;
};
