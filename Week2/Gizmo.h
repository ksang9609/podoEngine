#pragma once
#include "Vector.h"
#include "RenderInfo.h"
#include "TArray.h"
struct FGizmo {

	enum EGIZMO_AXIS
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
	float mHitRadius=0.25f; // 마우스 판정보정
	float mGizmoScale=1.0f;
	float mAxisLength = 1.0f;
	float mAxisThickness = 0.1f;
	EGIZMO_AXIS eAxis = NONE;
	EGIZMO_TYPE eType=TRANSLATE;

	FVector AxisDirection(EGIZMO_AXIS axis) const {
		switch (axis)
		{
		case X:  return FVector(1.0f, 0.0f, 0.0f);
		case Y:  return FVector(0.0f, 1.0f, 0.0f);
		case Z:  return FVector(0.0f, 0.0f, 1.0f);
		default: return FVector(0.0f, 0.0f, 0.0f);
		}
	}

	float mDragStartLocation;
	float mDragStartAxis;

	bool IsInGizmo(FVector nearPoint, FVector farPoint)
	{
		FVector norm_ray = (farPoint - nearPoint);
		norm_ray.Normalize();
		
		FVector w0 = nearPoint - mLocation;
		 //수학 함수 구현
	}

	TArray<FRenderInfo> GetGizmoRenderInfo() const
	{
		TArray<FRenderInfo> renderInfos;

		if (!mbVisible) return renderInfos;

		//기즈모타입을 확인후 타입에 맞는 모양을 리턴


		return renderInfos;
	}

	EPrimitive GetAxisPrimitive() const
	{
		switch (eType)
		{
		case TRANSLATE: return EPrimitive::EP_Cube;
		case ROTATE: return EPrimitive::EP_Cube; //EP_Rotate
		default: break;
		}
	}
	
};
