#pragma once

#include <cmath>

#include "Matrix.h"
#include "Core/Math/FBoundingBox.h"

enum class EContainment : uint8 { Outside, Intersect, Inside };
enum class EFrustumPlane : uint8 { Left, Right, Bottom, Top, Near, Far, Count };


struct FPlane
{
	FVector3 Normal;   // 안쪽을 향하는 단위 법선
	float    Offset = 0.0f;   // 원점에서 평면까지의 부호 있는 거리

	void Normalize()
	{
		const float length = Normal.Length();
		if (length <= 0.000001f) return;

		Normal = Normal * (1.0f / length);
		Offset /= length;
	}

	float SignedDistance(const FVector3& point) const
	{
		return FVector::dot(Normal, point) + Offset;
	}
};


struct FFrustum
{
	FPlane Planes[static_cast<int32> (EFrustumPlane::Count)];

	FPlane& Plane(EFrustumPlane p) { return Planes[static_cast<int32>(p)]; }
	const FPlane& Plane(EFrustumPlane p) const { return Planes[static_cast<int32>(p)]; }

	static FFrustum FrustumFromViewProjection(const FMatrix& M)
	{
		FFrustum result;

		// Row Vector 기준
		// ClipPosition = WorldPosition * ViewProjection
		// 그래서 Plane 추출 시 열을 사용한다.

		// Left : x + w >= 0
		result.Plane(EFrustumPlane::Left) =
		{
			{ M.M[0][0] + M.M[0][3],
			M.M[1][0] + M.M[1][3],
			M.M[2][0] + M.M[2][3] },
			M.M[3][0] + M.M[3][3]
		};

		// Right : w - x >= 0
		result.Plane(EFrustumPlane::Right) =
		{
			{ M.M[0][3] - M.M[0][0],
			M.M[1][3] - M.M[1][0],
			M.M[2][3] - M.M[2][0] },
			M.M[3][3] - M.M[3][0]
		};

		// Bottom : y + w >= 0
		result.Plane(EFrustumPlane::Bottom) =
		{
			{ M.M[0][1] + M.M[0][3],
			M.M[1][1] + M.M[1][3],
			M.M[2][1] + M.M[2][3] },
			M.M[3][1] + M.M[3][3]
		};

		// Top : w - y >= 0
		result.Plane(EFrustumPlane::Top) =
		{
			{ M.M[0][3] - M.M[0][1],
			M.M[1][3] - M.M[1][1],
			M.M[2][3] - M.M[2][1] },
			M.M[3][3] - M.M[3][1]
		};

		// Near : z >= 0
		result.Plane(EFrustumPlane::Near) =
		{
			{ M.M[0][2],
			M.M[1][2],
			M.M[2][2] },
			M.M[3][2]
		};

		// Far : w - z >= 0
		result.Plane(EFrustumPlane::Far) =
		{
			{ M.M[0][3] - M.M[0][2],
			M.M[1][3] - M.M[1][2],
			M.M[2][3] - M.M[2][2] },
			M.M[3][3] - M.M[3][2]
		};

		for (FPlane& plane : result.Planes)
		{
			plane.Normalize();
		}

		return result;
	}

	bool Intersects(const FBoundingBox& bounds) const
	{
		for (const FPlane& plane : Planes)
		{
			// Plane normal 방향으로 가장 멀리 있는 AABB 정점
			FVector3 positiveVertex;

			positiveVertex.x =
				plane.Normal.x >= 0.0f ? bounds.max.x : bounds.min.x;

			positiveVertex.y =
				plane.Normal.y >= 0.0f ? bounds.max.y : bounds.min.y;

			positiveVertex.z =
				plane.Normal.z >= 0.0f ? bounds.max.z : bounds.min.z;

			// Plane 방향으로 가장 멀리 있는 점조차 바깥이면
			// AABB 전체가 Frustum 밖
			if (plane.SignedDistance(positiveVertex) < 0.0f)
			{
				return false;
			}
		}

		return true;
	}

	EContainment Contains(const FBoundingBox& bounds) const {
		bool bIntersecting = false;

		for (const FPlane& plane : Planes) {
			FVector pv, nv;

			pv.x = plane.Normal.x >= 0.0f ? bounds.max.x : bounds.min.x;
			nv.x = plane.Normal.x >= 0.0f ? bounds.min.x : bounds.max.x;

			pv.y = plane.Normal.y >= 0.0f ? bounds.max.y : bounds.min.y;
			nv.y = plane.Normal.y >= 0.0f ? bounds.min.y : bounds.max.y;

			pv.z = plane.Normal.z >= 0.0f ? bounds.max.z : bounds.min.z;
			nv.z = plane.Normal.z >= 0.0f ? bounds.min.z : bounds.max.z;

			if (plane.SignedDistance(pv) < 0.0f) return EContainment::Outside;
			if (plane.SignedDistance(nv) < 0.0f) bIntersecting = true;	
		}

		return bIntersecting ? EContainment::Intersect : EContainment::Inside;
	}

};

