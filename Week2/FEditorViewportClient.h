#pragma once
#include "Vector.h"

#include <d3d11.h>
#include "World.h"
#include "Camera.h"
#include "RenderInfo.h"

struct FEditorViewportClient
{
public:
	void RayCast(D3D11_VIEWPORT ViewportInfo, UWorld* World);
	float GetFov() const { return mCamera.mFovDegree; }
	void Update(float deltaTime);
	bool IsMouseHit() const { return bMouseHit; }

	FCamera mCamera;

private:
	bool RayIntersectsTriangle( // 두개의 
		const FVector& Origin,
		const FVector& Dir,
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		float& OutT, float& OutU, float& OutV);

	void DeprojectScreenToWorld(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH, float NearZ, float FarZ,
		FVector& OutNearPoint, FVector& OutFarPoint);



	bool bMouseHit = false;

	//마우스 밑 무언가의 RenderInfo
	FRenderInfo HoveredRenderInfo;
};
