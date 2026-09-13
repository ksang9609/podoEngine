#pragma once

#include "Core/Container/TMap.h"

struct FCharacterInfo
{
	float U;
	float V;
	float UVWidth;
	float UVHeight;

	float Width;
	float Height;
	float Advance;
};

class FFontResource
{
public:
	const FCharacterInfo* FindCharacter(char character) const
	{
		return mCharacterMap.Find(character);
	}

	FFontResource() = default;

	// Build FFontResource from ASCII table. Each character is mapped to a grid cell in the texture atlas.
	FFontResource(int colums, int rows, float charWidth = 0.1f, float charHeight = 0.2f, float charAdvance = 0.7f)
	{
		for (int i = 0; i < colums * rows; ++i)
		{
			char character = static_cast<char>(i);
			FCharacterInfo info{};
			info.U = (i % colums) * (1.0f / colums);
			info.V = (i / colums) * (1.0f / rows);
			info.UVWidth = 1.0f / colums;
			info.UVHeight = 1.0f / rows;
			info.Width = charWidth;
			info.Height = charHeight;
			info.Advance = charAdvance;
			mCharacterMap.Add(character, info);
		}
	}

private:
	TMap<char, FCharacterInfo> mCharacterMap;
};
