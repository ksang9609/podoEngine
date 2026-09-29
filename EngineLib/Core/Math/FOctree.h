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

struct FOctreeBuildObject {
	FVector Center;
	float Radius;
};

class FOctree {
public:
	void Build(const TArray <FRenderInfo>& renderInfos);

private:
	void createRootNode(const TArray<FRenderInfo>& renderInfos);
	void insertAllObjects(const TArray<FRenderInfo>& renderInfos);
	bool outsideRoot(const FBoundingBox& box) const;

	void insertObject(uint32 nodeIndex, uint32 objectIndex, uint32 nodeDepth);
	void subDivide(uint32 nodeIndex);
	void flattenObjects();

private:
	TArray <FOctreeNode> mNodes;
	TArray <uint32> mInsideIndices;
	TArray <uint32> mOutsideObjects;

	TArray <FOctreeBuildObject> mBuildObject;	// 조회용
	TArray<TArray<uint32>> mBuildBuckets;		// 저장용

	static constexpr float margin = 1.5f;
	static constexpr int32 MaxObjectCount = 8;
	static constexpr uint32 MaxDepth = 8;
};
