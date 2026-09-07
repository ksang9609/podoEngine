#pragma once

#include "Matrix.h"
#include "Enum.h"

#include "TArray.h"
#include "TMap.h"
#include "Renderer.h"
#include "Camera.h"
#include "RenderInfo.h"

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

	void Prepare(bool bWireFrame, const FCamera* mCamera);
	//void Render(FTransform worldTransformMatrix, EPrimitive ePrimitive); // FRenderInfo
	void Render(const TArray<FRenderInfo> renderInfos); // FRenderInfo

	void Display();
	void Update(float deltaTime);

	float GetAspect() const { return mAspect; }

	// Todo: Change name
	void CreateBuffer(EPrimitive ePrimitive, FVertexSimple* vertices, uint32 verticesSize);
	URenderer* GetRenderer() const;

private:
	URenderer* mRenderer;
	FMatrix mViewProjectionMatrix;

	TMap<EPrimitive, FBuffer> mBufferMap;
	float mAspect;
};
