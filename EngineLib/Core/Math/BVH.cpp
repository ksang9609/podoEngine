// EngineLib/Core/Math/BVH.cpp

#include "BVH.h"

#include <algorithm>

#include "Core/Math/RayCast.h"

namespace
{
	FBoundingBox mergeBounds(
		const FBoundingBox& a,
		const FBoundingBox& b)
	{
		FBoundingBox result;

		for (int axis = 0; axis < 3; ++axis)
		{
			result.min[axis] = std::min(a.min[axis], b.min[axis]);
			result.max[axis] = std::max(a.max[axis], b.max[axis]);
		}
		return result;
	}

	FBoundingBox makeTriangleBounds(
		const FVector& v0,
		const FVector& v1,
		const FVector& v2)
	{
		FBoundingBox result{ v0, v0 };

		result = mergeBounds(result, FBoundingBox{ v1, v1 });
		result = mergeBounds(result, FBoundingBox{ v2, v2 });

		return result;
	}
}

void FMeshBVH::Build(
	const TArray<FNormalVertex>& vertices,
	const TArray<uint32>& indices,
	uint32 maxLeafSize
)
{
	Clear();

	if (indices.IsEmpty())
	{
		return;
	}
	if (indices.Num() % 3 != 0)
	{
		assert(false);
		return;
	}

	// Check the validity of the indices
	for (uint32 i = 0; i < indices.Num(); ++i)
	{
		if (indices[i] >= vertices.Num())
		{
			assert(false);
			return;
		}
	}

	maxLeafSize = std::max(1u, maxLeafSize);

	const uint32 triangleCount = indices.Num() / 3;

	TArray<FBVHBuildPrimitive> primitives;
	primitives.Reserve(triangleCount);

	for (uint32 triangle = 0; triangle < triangleCount; ++triangle)
	{
		const FVector& v0 = vertices[indices[triangle * 3 + 0]].pos;
		const FVector& v1 = vertices[indices[triangle * 3 + 1]].pos;
		const FVector& v2 = vertices[indices[triangle * 3 + 2]].pos;

		primitives.Add({
			makeTriangleBounds(v0, v1, v2), // Bounds
			(v0 + v1 + v2) / 3.0f,			// Center
			triangle						// TriangleIndex
			});
	}

	buildNode(primitives, 0, triangleCount, maxLeafSize);

	for (const FBVHBuildPrimitive& primitive : primitives)
	{
		mTriangleOrder.Add(primitive.TriangleIndex);
	}
}

bool FMeshBVH::Raycast(
	const FVector& localStart,
	const FVector& localEnd,
	const TArray<FNormalVertex>& vertices,
	const TArray<uint32>& indices,
	FRayTriangleHit& outHit,
	float tMax
) const
{
	assert(tMax >= 0.0f && tMax <= 1.0f);

	if (IsEmpty())
		return false;

	float enter, exit;

	// Check if the ray intersects the root node's bounding box
	if (!Raycast::IntersectSegmentAABB(
		localStart, localEnd, mNodes[0].Bounds,
		tMax, enter, exit))
	{
		return false;
	}

	float closestT = tMax;
	FRayTriangleHit hit;

	// Traverse the BVH starting from the root node
	if (!traverseNode(
		0, localStart, localEnd,
		vertices, indices, closestT, hit
	))
	{
		return false;
	}

	outHit = hit;
	return true;
}

bool FMeshBVH::traverseNode(
	int32 nodeIndex,
	const FVector& start,
	const FVector& end,
	const TArray<FNormalVertex>& vertices,
	const TArray<uint32>& indices,
	float& closestT,
	FRayTriangleHit& outHit) const
{
	const FBVHNode& node = mNodes[nodeIndex];

	// Check the intersection with the triangles
	if (node.IsLeaf())
	{
		bool found = false;

		// TODO: Use SIMD to accelerate triangle intersection tests
		for (uint32 i = node.First; i < node.First + node.Count; ++i)
		{
			const uint32 triangleIndex = mTriangleOrder[i];
			const uint32 base = triangleIndex * 3;

			const FVector& v0 = vertices[indices[base + 0]].pos;
			const FVector& v1 = vertices[indices[base + 1]].pos;
			const FVector& v2 = vertices[indices[base + 2]].pos;

			FRayTriangleHit hit;
			if (Raycast::IntersectSegmentTriangle(
				start, end, v0, v1, v2,
				closestT, hit))
			{
				closestT = hit.T;

				hit.TriangleIndex = triangleIndex;
				outHit = hit;
				found = true;
			}
		}

		return found;
	}

	int32 nearChild = node.LeftChildIndex;
	int32 farChild = node.RightChildIndex;

	float nearEnter, nearExit;
	float farEnter, farExit;

	const bool hitLeft = Raycast::IntersectSegmentAABB(
		start, end, mNodes[nearChild].Bounds,
		closestT, nearEnter, nearExit);

	const bool hitRight = Raycast::IntersectSegmentAABB(
		start, end, mNodes[farChild].Bounds,
		closestT, farEnter, farExit);

	if (!hitLeft && !hitRight)
	{
		return false;
	}

	if (!hitLeft)
	{
		return traverseNode(
			farChild, start, end,
			vertices, indices, closestT, outHit);
	}

	if (!hitRight)
	{
		return traverseNode(
			nearChild, start, end,
			vertices, indices, closestT, outHit);
	}

	if (farEnter < nearEnter)
	{
		std::swap(nearChild, farChild);
		std::swap(nearEnter, farEnter);
	}

	bool found = traverseNode(
		nearChild, start, end,
		vertices, indices, closestT, outHit);

	// closestT might have been updated,
	// so we need to check if we should traverse the far child
	if (farEnter <= closestT)
	{
		const bool farFound = traverseNode(
			farChild, start, end,
			vertices, indices, closestT, outHit);

		found = found || farFound;
	}

	return found;
}

void FMeshBVH::Clear()
{
	mNodes.Reset(0);
	mTriangleOrder.Reset(0);
}

bool FMeshBVH::IsEmpty() const
{
	return mNodes.IsEmpty();
}

int32 FMeshBVH::buildNode(
	TArray<FBVHBuildPrimitive>& primitives,
	uint32 first,
	uint32 count,
	uint32 maxLeafSize)
{
	FBoundingBox bounds = primitives[first].Bounds;

	FBoundingBox centerBounds{
		primitives[first].Center,
		primitives[first].Center
	};

	for (uint32 i = first + 1; i < first + count; ++i)
	{
		bounds = mergeBounds(bounds, primitives[i].Bounds);

		const FVector& center = primitives[i].Center;
		centerBounds = mergeBounds(
			centerBounds,
			FBoundingBox{ center, center }
		);
	}

	const int32 nodeIndex = static_cast<int32>(mNodes.Num());

	FBVHNode node;
	node.Bounds = bounds;
	mNodes.Add(node);


	if (count <= maxLeafSize)
	{
		mNodes[nodeIndex].First = first;
		mNodes[nodeIndex].Count = count;
		return nodeIndex;
	}

	// Determine the axis with the largest extent
	const FVector extent = centerBounds.max - centerBounds.min;

	int axis = 0;
	if (extent[1] > extent[axis]) axis = 1;
	if (extent[2] > extent[axis]) axis = 2;

	const uint32 mid = first + count / 2;

	std::nth_element(
		primitives.begin() + first,
		primitives.begin() + mid,
		primitives.begin() + first + count,
		[axis](const auto& a, const auto& b) -> bool
		{
			if (a.Center[axis] < b.Center[axis]) return true;
			if (a.Center[axis] > b.Center[axis]) return false;

			// If centers are equal, sort by triangle index to ensure deterministic order
			return a.TriangleIndex < b.TriangleIndex;
		}
	);

	const int32 left = buildNode(
		primitives, first, mid - first, maxLeafSize);

	const int32 right = buildNode(
		primitives, mid, first + count - mid, maxLeafSize);

	// Update the current node with child indices
	mNodes[nodeIndex].LeftChildIndex = left;
	mNodes[nodeIndex].RightChildIndex = right;

	return nodeIndex;
}
