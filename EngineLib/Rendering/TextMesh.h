#pragma once

#include "Core/Container/TArray.h"

#include "VertexType.h"

class FFontResource;

struct FTextMesh
{
	TArray<FVertexTextured> Vertices;
	TArray<uint32> Indices;

	uint32 TextNum = 0; // Number of characters in the text

	FTextMesh() = default;
	FTextMesh(const FString& text, const FFontResource& fontResource);
	void SetText(const FString& text, const FFontResource& fontResource);

private:
	// Helper function to generate the mesh for the given text
	void generateMesh(const FString& text, const FFontResource& fontResource);
};
