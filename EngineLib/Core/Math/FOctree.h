#pragma once
#include "Core/Core.h"
#include "Core/Container/TArray.h"
#include "Vector.h"
#include "Rendering/RenderInfo.h"
#include "FBoundingBox.h"

struct FOctreeNode {
	FVector Center;
	float HalfSize;

	uint32 ChildStart;
	uint32 ObjectStart;
	uint32 ObjectCount;
};

class FOctree {
public:
	void Build(const TArray <FRenderInfo>& renderInfos);

private:
	void createRootNode(const TArray<FRenderInfo>& renderInfos);
	void insertAllObjects(const TArray<FRenderInfo>& renderInfos);
	bool outsideRoot(const FBoundingBox& box) const;

private:
	TArray <FOctreeNode> mNodes;
	TArray <uint32> mInsideIndices;
	TArray <uint32> mOutsideObjects;

	static constexpr float margin = 1.5f;
};
