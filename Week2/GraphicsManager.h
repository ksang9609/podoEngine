#pragma once

#include "Matrix.h"
#include "Enum.h"

#include "TArray.h"
#include "TMap.h"
#include "Renderer.h"
#include "Camera.h"
#include "RenderInfo.h"
#include "Vector.h"

struct FBuffer
{
	ID3D11Buffer* Buffer;
	uint32 SourceNum;
};

class GraphicsManager
{
public:
	GraphicsManager(HWND hWindow);
	~GraphicsManager();

	void Prepare(const FCamera* mCamera);
	//void Render(FTransform worldTransformMatrix, EPrimitive ePrimitive); // FRenderInfo
	void Render(const TArray<FRenderInfo> renderInfos); // FRenderInfo
	void RenderOverlay(const TArray<FRenderInfo> renderInfos); //깊이버퍼 초기화
	void Display();
	void Update(float deltaTime);

	float GetAspect() const { return mAspect; }
	bool GetWireFrame() const { return mbWireFrame; } const
	void SetWireFrame(bool bWireFrame) { mbWireFrame = bWireFrame; }

	// Todo: Change name
	void CreateBuffer(EPrimitive ePrimitive, FVertexSimple* vertices, uint32 verticesSize);
	URenderer* GetRenderer() const;

	//Highlight
	static FVector GetPrimitiveCenter(EPrimitive type);
	static FVector GetPrimitiveHalfExtent(EPrimitive type);
	void RenderHighLight(const FRenderInfo& RI);

private:
	URenderer* mRenderer;
	FMatrix mViewProjectionMatrix;

	// Prepare에서 갱신. 하이라이트 두께의 픽셀 → 월드 환산에 쓴다
	FVector mCameraLocation;
	FVector mCameraForward;
	float mCameraFovDegree = 60.0f;

	TMap<EPrimitive, FBuffer> mBufferMap;

	// Graphics config
	bool mbWireFrame;
	float mAspect;

};
