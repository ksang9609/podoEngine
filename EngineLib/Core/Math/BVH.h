// EngineLib/Core/Math/BVH.h

#pragma once

#include "Core/Core.h"
#include "Core/Math/FBoundingBox.h"
#include "Core/Math/RayCast.h"
#include "Core/Container/TArray.h"

#include "Rendering/VertexType.h"

struct FBVHNode
{
	FBoundingBox Bounds;
	int32 LeftChildIndex = -1;  // -1 indicates no child
	int32 RightChildIndex = -1; // -1 indicates no child

	uint32 First = 0;
	uint32 Count = 0; // leaf node if Count > 0

	bool IsLeaf() const { return Count > 0; }
};

// Only used during BVH construction, not stored in the final BVH structure
struct FBVHBuildPrimitive
{
	FBoundingBox Bounds;
	FVector Center;
	uint32 TriangleIndex;
};

class FMeshBVH
{
public:
	void Build(
		const TArray<FNormalVertex>& vertices,
		const TArray<uint32>& indices,
		uint32 maxLeafSize = 4);

	bool Raycast(
		const FVector& localStart,
		const FVector& localEnd,
		const TArray<FNormalVertex>& vertices,
		const TArray<uint32>& indices,
		FRayTriangleHit& outHit,
		float tMax = 1.0f) const;

	void Clear();
	bool IsEmpty() const;


private:
	TArray<FBVHNode> mNodes;
	TArray<uint32> mTriangleOrder;

	int32 buildNode(
		TArray<FBVHBuildPrimitive>& primitives,
		uint32 first,
		uint32 count,
		uint32 maxLeafSize);

	bool traverseNode(
		int32 nodeIndex,
		const FVector& start,
		const FVector& end,
		const TArray<FNormalVertex>& vertices,
		const TArray<uint32>& indices,
		float& closestT,
		FRayTriangleHit& outHit) const;
};

