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
		SCALE,
	};


	FVector mLocation; // 기즈모의 위치

	// 드래그 기준값. 기즈모는 액터를 따라 움직이므로, 기준선을 시작 시점에 고정해 두지 않으면
	// 결과가 기준을 다시 움직여서 발산한다.
	FVector mDragStartLocation;       // 드래그 시작 시점의 액터 위치
	FVector mDragStartGizmoLocation;  // 드래그 시작 시점의 기즈모 위치 = 축 직선의 원점
	float mDragStartAxisS = 0.0f;     // 그 직선 위에서 처음 잡은 지점

	FMatrix TargetObjectTransformMatrix;
	bool mbVisible = false;
	bool mbHovered = false;
	float mGizmoScale=1.0f;
	float mAxisLength = mGizmoScale * 1.0f;
	float mAxisThickness = mGizmoScale * 0.2f;
	float mHitRadius= mAxisThickness*1.1f; // Translate 마우스 판정보정
	float mRingHitRadius = 0.3f; // Rotate마우스 판정보정 (+0.08배)
	float mGizmoSizeRatio = 0.2f;
	EGIZMO_AXIS eAxis = NONE; // 축위에 있는지
	EGIZMO_AXIS mDraggingAxis = NONE; // Drag중인 축
	EGIZMO_TYPE eType=ROTATE;

	FVector AxisDirection(EGIZMO_AXIS axis) const {
		switch (axis)
		{
		case X:  return FVector(1.0f, 0.0f, 0.0f);
		case Y:  return FVector(0.0f, 1.0f, 0.0f);
		case Z:  return FVector(0.0f, 0.0f, 1.0f);
		default: return FVector(0.0f, 0.0f, 0.0f);
		}
	}

	// 레이와 축 직선의 최단거리 지점을 축 파라미터 s로 돌려준다.
	bool GetClosestAxisParam(
		const FVector& nearPoint,
		const FVector& farPoint,
		const FVector& axisOrigin,
		EGIZMO_AXIS axis,
		float& outAxisS) const
	{
		FVector norm_ray = farPoint - nearPoint;
		norm_ray.Normalize();

		const FVector axisDir = AxisDirection(axis);
		const FVector w0 = nearPoint - axisOrigin;

		const float align = FVector::dot(norm_ray, axisDir);
		const float denom = 1.0f - align * align;
		if (FMath::Abs(denom) < 1e-5f) return false;   // 레이와 축이 거의 나란함

		const float rayProj = FVector::dot(norm_ray, w0);
		const float axisProj = FVector::dot(axisDir, w0);

		outAxisS = (axisProj - align * rayProj) / denom;

		return true;
	}

	// 축을 잡은 순간의 기준값을 저장한다. 이후 드래그는 전부 이 기준에 대한 상대량이다.
	void BeginDrag(const FVector& nearPoint, const FVector& farPoint, const FVector& actorLocation)
	{
		mDraggingAxis = eAxis;
		mDragStartLocation = actorLocation;
		mDragStartGizmoLocation = mLocation;
		mDragStartAxisS = 0.0f;

		GetClosestAxisParam(nearPoint, farPoint, mDragStartGizmoLocation, mDraggingAxis, mDragStartAxisS);
	}

	// 드래그 중인 축을 따라 액터가 있어야 할 위치. 축이 시선과 나란하면 false (이번 프레임은 건너뛴다)
	bool GetDragLocation(const FVector& nearPoint, const FVector& farPoint, FVector& outLocation) const
	{
		if (mDraggingAxis == NONE) return false;

		float axisS = 0.0f;
		if (!GetClosestAxisParam(nearPoint, farPoint, mDragStartGizmoLocation, mDraggingAxis, axisS))
		{
			return false;
		}

		outLocation = mDragStartLocation + AxisDirection(mDraggingAxis) * (axisS - mDragStartAxisS);

		return true;
	}

	bool IsRayInGizmo(FVector nearPoint, FVector farPoint)
	{

		/*
		Ray와 Axis사이의 최단거리를 구한다.
		3차원의 두 직선에 최단거리는 각 두 직선에 수직하는 선분이다.

		수직벡터 = 광선벡터 - 기즈모축벡터 
		1) W = D - A (모두 단위벡터임)
		2) WxD=0, WxA=0 (수직이므로 내적값이 0)

		3)W = ray시작점 + t*(ray단위벡터) - (기즈모시작점 + s*기즈모 단위벡터)

		*/
		mbHovered = false;
		eAxis = NONE;
		if (!mbVisible) return false;
		FVector norm_ray = (farPoint - nearPoint);
		norm_ray.Normalize(); // norm_ray= ray의 단위벡터
		const EGIZMO_AXIS axis[3] = { X, Y, Z };

		if (eType == ROTATE) //회전 기즈모의 충돌처리
		{
			float shortAxisLen = 0.0f;
			for (int i = 0; i < 3; ++i)
			{

				//t x norm_ray = 링위의점
				float t = FVector::dot((mLocation - nearPoint), AxisDirection(axis[i]))
					/ FVector::dot(norm_ray, AxisDirection(axis[i]));

				if (t < 0.0f) continue;

				FVector H = (norm_ray * t) + nearPoint;
				float r = (H-mLocation).Length(); //구 중심과 평면교점사이의 거리

				if (FMath::Abs(r - mGizmoScale) > mGizmoScale *mRingHitRadius) continue;

				if (eAxis == NONE || t < shortAxisLen) {
					shortAxisLen = t;
					eAxis = axis[i];
				}
			}


		}



		

		else if (eType == TRANSLATE) {
			FVector w0 = nearPoint - mLocation; // \
					 //수학 함수 구현
			const float axisLength = mAxisLength * mGizmoScale;
			const float hitRadius = mHitRadius * mGizmoScale;

			float bestRayT = 0.0f; //near point에서 광선방향으로 얼마나 이동했냐

			for (int i = 0; i < 3; ++i)
			{
				const FVector axisDir = AxisDirection(axis[i]);

			float axisS = 0.0f;
			if (!GetClosestAxisParam(nearPoint, farPoint, mLocation, axis[i], axisS)) continue;

			axisS = FMath::Clamp(axisS, 0.0f, axisLength);  // 무한 직선 → 선분

				const FVector axisPoint = mLocation + axisDir * axisS; // 현재위치에서 기즈모방향으로 얼만큼 이동했나

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
		}
		return eAxis != NONE;
	}

	void Reset()
	{
		mbVisible = false;
		mLocation = FVector(0.0f, 0.0f, 0.0f);

		// 드래그 상태도 같이 지운다. 안 그러면 선택이 풀린 뒤에도 드래그가 살아남는다
		eAxis = NONE;
		mDraggingAxis = NONE;
	}

	EPrimitive GetAxisPrimitive() const
	{
		switch (eType)
		{
		case TRANSLATE: return EPrimitive::EP_GizmoArrow;
		case ROTATE: return EPrimitive::EP_CirCle; //EP_Rotate
		default: return EPrimitive::EP_GizmoArrow;
		}
	}

	FMatrix GetAxisMatrix(EGIZMO_AXIS axis) const // 축모양 도형을 반환
	{
		const float length = mAxisLength * mGizmoScale;
		const float thickness = mAxisThickness * mGizmoScale;

		const FRotator rotation = FRotator::FromDirection(AxisDirection(axis));
		if (eType == ROTATE)
		{
			const EGIZMO_AXIS axis[3] = { X, Y, Z };
			for (int i = 0;i < 3;i++) {
				return FMatrix::Scale(FVector(mGizmoScale))
					* FMatrix::Rotate(rotation)
					* FMatrix::Translation(mLocation);
			}
		}
		else if(eType == ROTATE) { //eType= Translate
			return FMatrix::Scale(FVector(length, thickness, thickness))
				* FMatrix::Translation(FVector(0.0f, -thickness * 0.5f, -thickness * 0.5f)) // 긴막대기 모양으로변환
				* FMatrix::Rotate(rotation)
				* FMatrix::Translation(mLocation);
		}
	}

	FVector4 GetAxisColor(EGIZMO_AXIS axis) const
	{
		float alpha =1.0f;
		if (axis == eAxis) return FVector4(1.0f, 1.0f, 1.0f, 1.0f);   // 마우스가 올라간 축

		switch (axis)
		{
		case X:  return FVector4(1.0f, 0.0f, 0.0f, alpha);
		case Y:  return FVector4(0.0f, 1.0f, 0.0f, alpha);
		case Z:  return FVector4(0.0f, 0.0f, 1.0f, alpha);
		default: return FVector4(0.0f, 0.0f, 0.0f, alpha);
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
			renderInfos.Add({ GetAxisPrimitive(), GetAxisMatrix(axis[i]),FObjectID{},GetAxisColor(axis[i])});
		}
		if (eType == ROTATE)
		{
			renderInfos.Add({ EPrimitive::EP_Sphere,
				  FMatrix::Scale(FVector(0.96f * mGizmoScale)) * FMatrix::Translation(mLocation),
				  FObjectID{},
				  FVector4(1.0f, 1.0f, 1.0f, 1.0f) });   // 어두운 회색
		}
	

		return renderInfos;
	}

	void Update(
		const AActor* targetActor,
		const FVector& cameraLocation,
		const FVector& cameraForward,
		float fovDegree) // Gizmo 깊이에따른 원근크기 보정
	{
		if (!targetActor)
		{
			mbVisible = false;
			return;
		}

		mbVisible = true;
		mLocation = targetActor->GetTransform().Location;

		float depth = FVector::dot(mLocation - cameraLocation, cameraForward);
		const float tanHalfFov = tanf(FMath::DegreesToRadians(fovDegree * 0.5f));
		mGizmoScale = depth * tanHalfFov * mGizmoSizeRatio;
	}

};
