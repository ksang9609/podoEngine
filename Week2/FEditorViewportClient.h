#pragma once
#include "Vector.h"
struct FEditorViewportClient
{
	bool RayIntersectsTriangle(
		const FVector& Origin,
		const FVector& Dir,
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		float& OutT, float& OutU, float& OutV) {

		const float EPSILON = 1e-6f;

		//삼각형판정 => O +tD = V0+ uE1+vE2
		// -tD + uE1 + vE2 = O - V0
		//E2=v2-v0. E1=v1-v0

		FVector T = Origin - V0;
		FVector E2 = V2 - V0;
		FVector E1 = V1 - V0;

		FVector P = FVector::cross(Dir, E2);
		float Det = FVector::dot(E1, P);

		if (fabsf(Det) < EPSILON) return false;   // 평면과 평행

		float InvDet = 1.0f / Det;
\
		OutU = FVector::dot(T, P) * InvDet;
		if (OutU < 0.0f || OutU > 1.0f) return false;

		FVector Q = FVector::cross(T, E1);
		OutV = FVector::dot(Dir, Q) * InvDet;
		if (OutV < 0.0f || OutU + OutV > 1.0f) return false;

		OutT = FVector::dot(E2, Q) * InvDet;

		return (OutT > EPSILON);                  // 광선 앞쪽만

	}
};
