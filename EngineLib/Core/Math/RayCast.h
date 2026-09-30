// EngineLib/Core/Math/RayCast.h

#pragma once

#include "Core/Core.h"
#include "Core/Math/Vector.h"
#include "Core/Math/FBoundingBox.h"

struct FRayTriangleHit
{
	// P(t) = RayStart + t * (RayEnd - RayStart)
	float T = 1.0f;

	// barycentric coordinates
	float U = 0.0f;
	float V = 0.0f;

	// -1 if no triangle information is available
	int32 TriangleIndex = -1;
};

namespace Raycast
{
	// Slab algorithm for ray-AABB intersection.
	inline bool IntersectSegmentAABB(
		const FVector& rayStart,
		const FVector& rayEnd,
		const FBoundingBox& bounds,
		float tMax,
		float& outEnter,
		float& outExit)
	{
		const FVector direction = rayEnd - rayStart;

		float enter = 0.0f;
		float exit = tMax;

		for (int axis = 0; axis < 3; ++axis)
		{
			const float origin = rayStart[axis];
			const float dir = direction[axis];

			// Handle the case where the ray is parallel to the slab
			const float minValue = bounds.min[axis];
			const float maxValue = bounds.max[axis];
			if (fabsf(dir) < 1e-6f)
			{
				if (origin < minValue || origin > maxValue)
				{
					return false;
				}
				continue;
			}

			float t0 = (minValue - origin) / dir;
			float t1 = (maxValue - origin) / dir;
			if (t0 > t1)
			{
				std::swap(t0, t1);
			}

			enter = std::max(enter, t0);
			exit = std::min(exit, t1);

			if (enter > exit)
			{
				return false;
			}
		}

		outEnter = enter;
		outExit = exit;
		return true;
	}

	inline bool IntersectSegmentAABB(
		const FVector& rayStart,
		const FVector& rayEnd,
		const FBoundingBox& bounds,
		float tMax)
	{
		float enter, exit;
		return IntersectSegmentAABB(rayStart, rayEnd, bounds, tMax, enter, exit);
	}

	inline bool IntersectSegmentTriangle(
		const FVector& rayStart,
		const FVector& rayEnd,
		const FVector& v0,
		const FVector& v1,
		const FVector& v2,
		float tMax,
		FRayTriangleHit& outHit)
	{
		constexpr float episilon = 1e-6f;

		// Möller–Trumbore intersection algorithm
		// Ray: P(t) = rayStart + t * (rayEnd - rayStart)
		// Triangle: v0, v1, v2
		// rayStart + t * (rayEnd - rayStart) = v0 + u * (v1 - v0) + v * (v2 - v0)

		const FVector direction = rayEnd - rayStart;
		const FVector edge1 = v1 - v0;
		const FVector edge2 = v2 - v0;

		const FVector p = FVector::cross(direction, edge2);
		const float determinant = FVector::dot(edge1, p);

		if (fabsf(determinant) < episilon)
		{
			return false; // Ray is parallel to the triangle
		}

		const float inverseDeterminant = 1.0f / determinant;
		const FVector offset = rayStart - v0;

		const float u = FVector::dot(offset, p) * inverseDeterminant;
		if (u < 0.0f || u > 1.0f)
		{
			return false; // Intersection is outside the triangle
		}

		const FVector q = FVector::cross(offset, edge1);

		const float v = FVector::dot(direction, q) * inverseDeterminant;
		if (v < 0.0f || u + v > 1.0f)
		{
			return false; // Intersection is outside the triangle
		}

		const float t = FVector::dot(edge2, q) * inverseDeterminant;
		if (t < 0.0f || t > tMax)
		{
			return false; // Intersection is outside the segment
		}

		outHit.T = t;
		outHit.U = u;
		outHit.V = v;
		return true;
	}
}
