#pragma once

struct FName
{
	FName();
	FName(const char* pStr);
	FName(const FString& str);

	FString ToString() const;

	int32 Compare(const FName& other) const;
	bool operator==(const FName& other) const;

	int32 DisplayIndex;
	int32 ComparisonIndex;
};
