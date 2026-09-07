#pragma once
#include "Vector.h"
#include "RenderInfo.h"
#include "TArray.h"
struct FGizmo {

	enum EGIZMO_AXIS //어떤축이 선택되었는지
	{
		NONE,
		X,
		Y,
		Z
	};
	
	enum EGIZMO_TYPE {
		TRANSLATE,
		ROTATE,
	};


	FVector mLocation; // 기즈모의 위치
	FMatrix TargetObjectTransformMatrix;
	bool mbVisible = false;
	bool bMouseOnGizmo = false;
	float mGizmoScale=1.0f;
	float mAxisLength = mGizmoScale * 1.0f;
	float mAxisThickness = mGizmoScale * 0.2f;
	float mHitRadius= mAxisThickness*1.1f; // 마우스 판정보정
	float mGizmoSizeRatio = 0.2f;
	EGIZMO_AXIS eAxis = NONE; // 축위에 있는지
	EGIZMO_TYPE eType=TRANSLATE;
	EGIZMO_AXIS mDraggingAxis = NONE; // Drag중인 축

	FVector AxisDirection(EGIZMO_AXIS axis) const {
		switch (axis)
		{
		case X:  return FVector(1.0f, 0.0f, 0.0f);
		case Y:  return FVector(0.0f, 1.0f, 0.0f);
		case Z:  return FVector(0.0f, 0.0f, 1.0f);
		default: return FVector(0.0f, 0.0f, 0.0f);
		}
	}

	FVector mDragStartLocation;
	float mDragStartAxis;

	bool IsRayInGizmo(FVector nearPoint, FVector farPoint)
	{
		eAxis = NONE;
		if (!mbVisible) return false;
		FVector norm_ray = (farPoint - nearPoint);
		norm_ray.Normalize();
		
		FVector w0 = nearPoint - mLocation;
		 //수학 함수 구현

		const float axisLength = mAxisLength * mGizmoScale;
		const float hitRadius = mHitRadius * mGizmoScale;

		const EGIZMO_AXIS axis[3] = { X, Y, Z };
		float bestRayT = 0.0f;

		for (int i = 0; i < 3; ++i)
		{
			const FVector axisDir = AxisDirection(axis[i]);

			const float align = FVector::dot(norm_ray, axisDir);
			const float rayProj = FVector::dot(norm_ray, w0);
			const float axisProj = FVector::dot(axisDir, w0);

			const float denom = 1.0f - align * align;
			if (FMath::Abs(denom) < 1e-5f) continue;        // 레이와 축이 거의 나란함

			float axisS = (axisProj - align * rayProj) / denom;
			axisS = FMath::Clamp(axisS, 0.0f, axisLength);  // 무한 직선 → 선분

			const FVector axisPoint = mLocation + axisDir * axisS;

			const float rayT = FVector::dot(norm_ray, axisPoint - nearPoint);
			if (rayT < 0.0f) continue;                      // 카메라 뒤쪽

			const FVector rayPoint = nearPoint + norm_ray * rayT;
			const float distance = (rayPoint - axisPoint).Length();

			if (distance > hitRadius) continue;             // 캡슐 밖

			if (eAxis == NONE || rayT < bestRayT)           // 겹치면 카메라에 가까운 축
			{
				bestRayT = rayT;
				eAxis = axis[i];
			}
		}

		return eAxis != NONE;
	}

	EPrimitive GetAxisPrimitive() const
	{
		switch (eType)
		{
		case TRANSLATE: return EPrimitive::EP_GizmoArrow;
		case ROTATE: return EPrimitive::EP_Cube; //EP_Rotate
		default: break;
		}
	}

	FMatrix GetAxisMatrix(EGIZMO_AXIS axis) const // 축모양 도형을 반환
	{
		const float length = mAxisLength * mGizmoScale;
		const float thickness = mAxisThickness * mGizmoScale;

		const FRotator rotation = FRotator::FromDirection(AxisDirection(axis));

		return FMatrix::Scale(FVector(length, thickness, thickness))
			* FMatrix::Translation(FVector(0.0f, -thickness * 0.5f, -thickness * 0.5f)) // 긴막대기 모양으로변환
			* FMatrix::Rotate(rotation)
			* FMatrix::Translation(mLocation);
	}

	FVector4 GetAxisColor(EGIZMO_AXIS axis) const
	{
		if (axis == eAxis) return FVector4(1.0f, 1.0f, 0.0f, 1.0f);   // 마우스가 올라간 축

		switch (axis)
		{
		case X:  return FVector4(1.0f, 0.0f, 0.0f, 1.0f);
		case Y:  return FVector4(0.0f, 1.0f, 0.0f, 1.0f);
		case Z:  return FVector4(0.0f, 0.4f, 1.0f, 1.0f);
		default: return FVector4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}


	TArray<FRenderInfo> GetGizmoRenderInfo() const // Gizmo 모형 렌더정보
	{
		TArray<FRenderInfo> renderInfos;

		if (!mbVisible) return renderInfos;
		const EGIZMO_AXIS axis[3] = { X, Y, Z };
		//기즈모타입을 확인후 타입에 맞는 모양을 리턴
		for (int i = 0; i < 3; ++i)
		{
			if(axis[i]==eAxis) { /* highlight */ }
			renderInfos.Add({ GetAxisPrimitive(), GetAxisMatrix(axis[i]),FObjectID{},GetAxisColor(axis[i])});
		}

		return renderInfos;
	}

	void Update(const FVector& cameraLocation, const FVector& cameraForward, float fovDegree) // Gizmo 깊이에따른 원근크기 보정
	{
		float depth = FVector::dot(mLocation - cameraLocation, cameraForward);
		const float tanHalfFov = tanf(FMath::DegreesToRadians(fovDegree * 0.5f));
		mGizmoScale = depth * tanHalfFov * mGizmoSizeRatio;
	}

};
