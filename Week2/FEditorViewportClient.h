#pragma once
#include "Vector.h"

#include <d3d11.h>
#include "World.h"
#include "Camera.h"
#include "RenderInfo.h"
#include "Gizmo.h"

class AActor;

struct FEditorViewportClient
{
public:
	void RayCast(D3D11_VIEWPORT ViewportInfo, UWorld* World);
	float GetFov() const { return mCamera.mFovDegree; }
	void Update(float deltaTime);
	bool IsMouseHit() const { return bMouseHit; }

	//마우스 밑 무언가의

	FRenderInfo HoveredRenderInfo;
	FRenderInfo ClickedRenderInfo;

	//마우스 밑 무언가가 Actor이면 저장. RayCast 에서 채워야 함 (아직 미구현)
	AActor* ClickedActor = nullptr;

	FCamera& GetCamera() { return mCamera; }


	FCamera mCamera;
	FGizmo mGizmo;

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
};
