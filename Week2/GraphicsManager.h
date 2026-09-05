#pragma once

#include "Matrix.h"
#include "Enum.h"

#include "TMap.h"
#include "Renderer.h"
#include "Transform.h"
#include "Camera.h"

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

	void Prepare(bool bWireFrame);
	void Render(FTransform worldTransformMatrix, EPrimitive ePrimitive);
	void Display();

	// Todo: Change name
	void CreateBuffer(EPrimitive ePrimitive, FVertexSimple* vertices, uint32 verticesSize);

	URenderer* GetRenderer() const;

private:
	URenderer* mRenderer;
	FCamera* mCamera;
	FMatrix mViewProjectionMatrix;

	TMap<EPrimitive, FBuffer> mBufferMap;

	float mAspect;
	float mFovRad;
};
