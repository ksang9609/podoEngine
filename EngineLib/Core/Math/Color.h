#pragma once

#include "Core/Archive/StructuredArchive.h"

struct FLinearColor
{
	float R;
	float G;
	float B;
	float A;

	void Serialize(FStructuredArchive& archive)
	{
		archive << TNamedValue<float>{ "R", R };
		archive << TNamedValue<float>{ "G", G };
		archive << TNamedValue<float>{ "B", B };
		archive << TNamedValue<float>{ "A", A };
	}
};
