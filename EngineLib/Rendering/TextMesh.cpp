#include "TextMesh.h"

#include "Editor/Console.h"
#include "FontResource.h"

FTextMesh::FTextMesh(const FString& text, const FFontResource& fontResource)
{
	generateMesh(text, fontResource);
}

void FTextMesh::SetText(const FString& text, const FFontResource& fontResource)
{
	generateMesh(text, fontResource);
}

void FTextMesh::generateMesh(const FString& text, const FFontResource& fontResource)
{
	TextNum = text.Len();

	// Variables for center alignment
	float textTotalWidth = 0.0f;
	float textMaxHeight = 0.0f;

	Vertices.Reset(TextNum * 4); // 4 vertices per character
	Indices.Reset(TextNum * 6);  // 6 indices per character (2 triangles)

	uint32 i = 0;
	for (char ch : text)
	{
		const FCharacterInfo* charInfo = fontResource.FindCharacter(ch);

		if (charInfo == nullptr)
		{
			UE_LOG(Warning, Render, "Character '%c' not found in font resource.", ch);
			continue;
		}

		// UV coordinates
		const float u0 = charInfo->U;
		const float u1 = charInfo->U + charInfo->UVWidth;
		const float v0 = charInfo->V;
		const float v1 = charInfo->V + charInfo->UVHeight;

		// Local position of the character quad
		const float advance = charInfo->Width * charInfo->Advance;
		const float left = static_cast<float>(i) * advance;
		const float right = left + charInfo->Width;
		const float top = 0.0f;
		const float bottom = top - charInfo->Height;

		// Update total width and max height
		textTotalWidth += advance;
		textMaxHeight = FMath::Max(textMaxHeight, charInfo->Height);

		// Add vertices for the character quad
		Vertices.Add({ 0.0f, left, top, u0, v0 });
		Vertices.Add({ 0.0f, right, top, u1, v0 });
		Vertices.Add({ 0.0f, right, bottom, u1, v1 });
		Vertices.Add({ 0.0f, left, bottom, u0, v1 });

		// Add indices for the character quad
		uint32 baseIndex = i * 4;
		Indices.Add(baseIndex + 0); // Triangle 1
		Indices.Add(baseIndex + 1);
		Indices.Add(baseIndex + 2);
		Indices.Add(baseIndex + 2); // Triangle 2
		Indices.Add(baseIndex + 3);
		Indices.Add(baseIndex + 0);

		++i;
	}

	// Center the text mesh around the origin
	float offsetX = -textTotalWidth * 0.5f;
	float offsetY = textMaxHeight * 0.5f;

	for (FVertexTextured& vertex : Vertices)
	{
		vertex.y += offsetX;
		vertex.z += offsetY;
	}
}
