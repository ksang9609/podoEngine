// TexturedPrimitives.h
#pragma once

#include "Renderer.h" // FVertexTextured 정의

inline FVertexTextured QuadTextureVertices[] =
{
	// x      y      z       u     v
	{ -0.5f, -0.5f, 0.0f,   0.0f, 1.0f },
	{ -0.5f,  0.5f, 0.0f,   0.0f, 0.0f },
	{ 0.5f,  0.5f, 0.0f,   1.0f, 0.0f },

	{ -0.5f, -0.5f, 0.0f,   0.0f, 1.0f },
	{ 0.5f,  0.5f, 0.0f,   1.0f, 0.0f },
	{ 0.5f, -0.5f, 0.0f,   1.0f, 1.0f },
};

// 삼각형: 정점 3개
inline FVertexTextured TriangleTextureVertices[] =
{
	// x      y      z       u     v
	{ -0.5f, -0.5f, 0.0f,   0.0f, 1.0f },
	{ 0.0f,  0.5f, 0.0f,   0.5f, 0.0f },
	{ 0.5f, -0.5f, 0.0f,   1.0f, 1.0f },
};

// 중심 (0, 0, 0), 한 변의 길이 1
inline FVertexTextured CubeTextureVertices[] =
{
	// x      y      z       u     v

	// -Z 면
	{ -0.5f, -0.5f, -0.5f,  0.0f, 1.0f },
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f },
	{ 0.5f,  0.5f, -0.5f,  1.0f, 0.0f },

	{ -0.5f, -0.5f, -0.5f,  0.0f, 1.0f },
	{ 0.5f,  0.5f, -0.5f,  1.0f, 0.0f },
	{ 0.5f, -0.5f, -0.5f,  1.0f, 1.0f },

	// +Z 면
	{ 0.5f, -0.5f,  0.5f,  0.0f, 1.0f },
	{ 0.5f,  0.5f,  0.5f,  0.0f, 0.0f },
	{ -0.5f,  0.5f,  0.5f,  1.0f, 0.0f },

	{ 0.5f, -0.5f,  0.5f,  0.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f,  1.0f, 0.0f },
	{ -0.5f, -0.5f,  0.5f,  1.0f, 1.0f },

	// -X 면
	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f,  0.0f, 0.0f },
	{ -0.5f,  0.5f, -0.5f,  1.0f, 0.0f },

	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f },
	{ -0.5f,  0.5f, -0.5f,  1.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f,  1.0f, 1.0f },

	// +X 면
	{ 0.5f, -0.5f, -0.5f,  0.0f, 1.0f },
	{ 0.5f,  0.5f, -0.5f,  0.0f, 0.0f },
	{ 0.5f,  0.5f,  0.5f,  1.0f, 0.0f },

	{ 0.5f, -0.5f, -0.5f,  0.0f, 1.0f },
	{ 0.5f,  0.5f,  0.5f,  1.0f, 0.0f },
	{ 0.5f, -0.5f,  0.5f,  1.0f, 1.0f },

	// +Y 면
	{ -0.5f,  0.5f, -0.5f,  0.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f,  0.0f, 0.0f },
	{ 0.5f,  0.5f,  0.5f,  1.0f, 0.0f },

	{ -0.5f,  0.5f, -0.5f,  0.0f, 1.0f },
	{ 0.5f,  0.5f,  0.5f,  1.0f, 0.0f },
	{ 0.5f,  0.5f, -0.5f,  1.0f, 1.0f },

	// -Y 면
	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f },
	{ -0.5f, -0.5f, -0.5f,  0.0f, 0.0f },
	{ 0.5f, -0.5f, -0.5f,  1.0f, 0.0f },

	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f },
	{ 0.5f, -0.5f, -0.5f,  1.0f, 0.0f },
	{ 0.5f, -0.5f,  0.5f,  1.0f, 1.0f },
};


inline void BuildCubeAtlasVertices(FVertexTextured(&outVertices)[36])
{
	// 기존 면 순서: -Z, +Z, -X, +X, +Y, -Y
	// 각 면에 배정할 아틀라스 칸 번호
/*	const int faceCells[6] = { 0, 1, 2, 3, 4, 5 };

	for (int face = 0; face < 6; ++face)
	{
		const int cellIndex = faceCells[face];
		const int column = cellIndex % 3;
		const int row = cellIndex / 3;

		for (int vertex = 0; vertex < 6; ++vertex)
		{
			const int index = face * 6 + vertex;

			// 위치와 기존 UV 복사
			outVertices[index] = CubeTextureVertices[index];

			// UV만 선택한 칸 안으로 변환
			outVertices[index].u =	(column + CubeTextureVertices[index].u) / 3.0f;

			outVertices[index].v = (row + CubeTextureVertices[index].v) / 2.0f;
		}
	}*/

	const int faceCells[6] = { 5, 4, 3, 0, 2, 1 };

	for (int face = 0; face < 6; ++face)
	{
		const int column = faceCells[face] % 3;
		const int row = faceCells[face] / 3;

		for (int vertex = 0; vertex < 6; ++vertex)
		{
			const int index = face * 6 + vertex;
			const FVertexTextured& source =
				CubeTextureVertices[index];

			outVertices[index] = source;

			float u = source.u;
			float v = source.v;

			switch (face)
			{
			case 0: // -Z: Bottom
				u = source.v;
				v = 1.0f - source.u;
				break;

			case 1: // +Z: Top
				u = 1.0f - source.v;
				v = source.u;
				break;

			case 2: // -X: Back — 기존 유지
				u = 1.0f - source.v;
				v = source.u;
				break;

			case 3: // +X: Front — 기존 유지
				u = source.v;
				v = 1.0f - source.u;
				break;

			case 5: // -Y: Left — 기존 유지
				u = 1.0f - source.u;
				v = 1.0f - source.v;
				break;

				// +Y: Right — 기존 유지
			}

			outVertices[index].u = (column + u) / 3.0f;
			outVertices[index].v = (row + v) / 2.0f;
		}
	}
}
