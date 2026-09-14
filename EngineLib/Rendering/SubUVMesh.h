#pragma once

#include "Core/Container/TArray.h"

#include "VertexType.h"

struct FSubUVMesh
{
	TArray<FVertexTextured> Vertices;
	TArray<uint32> Indices;

	FSubUVMesh() = default;
	FSubUVMesh(float n, float m);

	void UpdateMesh(float n, float m);

private:
	// Generate the mesh from i-th item of n by m grid
	void generateMesh(float n, float m, float i);
};
