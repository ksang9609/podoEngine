#pragma once

#include "Core/Container/TArray.h"

#include "VertexType.h"

struct FSubUVMesh
{
	TArray<FVertexTextured> Vertices;
	TArray<uint32> Indices;

	FSubUVMesh() = default;
	FSubUVMesh(uint32 n, uint32 m);

	void UpdateMesh(uint32 n, uint32 m, uint32 i);

private:
	// Generate the mesh from i-th item of n by m grid
	void generateMesh(uint32 n, uint32 m, uint32 i);
};
