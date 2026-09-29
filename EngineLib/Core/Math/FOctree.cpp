#include "FOctree.h"

void FOctree::createRootNode(const TArray<const FRenderInfo*>& renderInfos)
{
	FVector sceneMin = FLT_MAX;
	FVector sceneMax = -FLT_MAX;

	for (const FRenderInfo* renderInfo : renderInfos) {
		sceneMin.x = std::min(sceneMin.x, renderInfo->WorldBounds.min.x);
		sceneMin.y = std::min(sceneMin.y, renderInfo->WorldBounds.min.y);
		sceneMin.z = std::min(sceneMin.z, renderInfo->WorldBounds.min.z);
		sceneMax.x = std::max(sceneMax.x, renderInfo->WorldBounds.max.x);
		sceneMax.y = std::max(sceneMax.y, renderInfo->WorldBounds.max.y);
		sceneMax.z = std::max(sceneMax.z, renderInfo->WorldBounds.max.z);
	}

	FOctreeNode rootNode;
	rootNode.ChildStart = 0;
	rootNode.ObjectStart = 0;
	rootNode.ObjectCount = 0;

	FVector extent = sceneMax - sceneMin;
	const float maxExtent = (std::max)(extent.x, (std::max)(extent.y, extent.z));
	rootNode.HalfSize = maxExtent * 0.5f * margin;
	rootNode.Center = (sceneMax + sceneMin) * 0.5f;

	mNodes.Add(rootNode);
	mBuildBuckets.Add(TArray<uint32>());
}

void FOctree::insertAllObjects(const TArray<const FRenderInfo*>& renderInfos)
{
	for (int32 i = 0; i < renderInfos.Num(); i++) {
		const FBoundingBox& box = renderInfos[i]->WorldBounds;

		const FVector half = (box.max - box.min) * 0.5f;

		mBuildObject[i].Center = (box.min + box.max) * 0.5f;
		mBuildObject[i].Radius = (std::max)(half.x, (std::max)(half.y, half.z));

		if (outsideRoot(box)) {
			mOutsideObjects.Add(i);
			continue;
		}

		insertObject(0, i, 0);
	}
	flattenObjects();
}

bool FOctree::outsideRoot(const FBoundingBox& box) const
{
	const FVector objectCenter = (box.min + box.max) * 0.5f;
	const FVector halfExtent = (box.max - box.min) * 0.5f;
	const float objectRadius = (std::max)(halfExtent.x, (std::max)(halfExtent.y, halfExtent.z));

	if (objectRadius > mNodes[0].HalfSize) return true;

	if (fabsf(objectCenter.x - mNodes[0].Center.x) > mNodes[0].HalfSize
		|| fabsf(objectCenter.y - mNodes[0].Center.y) > mNodes[0].HalfSize
		|| fabsf(objectCenter.z - mNodes[0].Center.z) > mNodes[0].HalfSize)
		return true;

	return false;
}

void FOctree::insertObject(uint32 nodeIndex, uint32 objectIndex, uint32 nodeDepth)
{
	const bool bIsLeaf = (mNodes[nodeIndex].ChildStart == 0);

	// 오브젝트 지름이 더 크면 ㅈㅈ 
	if (mBuildObject[objectIndex].Radius > mNodes[nodeIndex].HalfSize * 0.5f) {
		mBuildBuckets[nodeIndex].Add(objectIndex);
		return;
	}

	// 오브젝트 깊이가 더 앞에있다면 ㅈㅈ 
	if (nodeDepth >= MaxDepth) {
		mBuildBuckets[nodeIndex].Add(objectIndex);
		return;
	}

	//  리프며 더 담을 수 있다면 ㅈㅈ 
	if (bIsLeaf && mBuildBuckets[nodeIndex].Num() < MaxObjectCount) {
		mBuildBuckets[nodeIndex].Add(objectIndex);
		return;
	}

	// 리프인데 꽉찼다면 일단 쪼개기 
	if (bIsLeaf) {
		subDivide(nodeIndex);
	}

	// 리프꽉 or 이미 내부노드(자식o)면 통과시키기
	const FVector& nodeCenter = mNodes[nodeIndex].Center;
	const uint8 octant = (mBuildObject[objectIndex].Center.x >= nodeCenter.x ? 1 : 0)
		+ (mBuildObject[objectIndex].Center.y >= nodeCenter.y ? 2 : 0)
		+ (mBuildObject[objectIndex].Center.z >= nodeCenter.z ? 4 : 0);

	insertObject(mNodes[nodeIndex].ChildStart + octant, objectIndex, nodeDepth+1);
	return;
} 

void FOctree::subDivide(uint32 nodeIndex)
{
	const float childHalfSize = mNodes[nodeIndex].HalfSize * 0.5f;
	const FVector parentCenter = mNodes[nodeIndex].Center;

	mNodes[nodeIndex].ChildStart = mNodes.Num();


	for (uint32 i = 0; i < 8; i++) {
		FOctreeNode child;

		child.Center.x = parentCenter.x + ((i & (1 << 0)) ? childHalfSize : -childHalfSize);
		child.Center.y = parentCenter.y + ((i & (1 << 1)) ? childHalfSize : -childHalfSize);
		child.Center.z = parentCenter.z + ((i & (1 << 2)) ? childHalfSize : -childHalfSize);
		child.HalfSize = childHalfSize;
		child.ChildStart = 0;	// 새 생성 직후이므로 자식 없어서 0
		child.ObjectCount = 0;
		child.ObjectStart = 0;

		mNodes.Add(child);
		mBuildBuckets.Add(TArray<uint32>());
	}
}

void FOctree::flattenObjects() {
	// 뒤죽박죽인 mNodes를 mInsideIndices에 예쁘게 정렬하기(평탄화)
	mInsideIndices.Reset(0);

	for (int i = 0; i < mNodes.Num(); i++) {
		mNodes[i].ObjectStart = mInsideIndices.Num();
		mNodes[i].ObjectCount = mBuildBuckets[i].Num();

		for (auto object : mBuildBuckets[i]) {
			mInsideIndices.Add(static_cast<uint32> (object));
		}
	}

	mBuildObject.Reset(0);
	mBuildBuckets.Reset(0);
}

void FOctree::Build(const TArray<const FRenderInfo*>& renderInfos)
{
	mNodes.Reset(0);
	mInsideIndices.Reset(0);
	mOutsideObjects.Reset(0);
	mBuildObject.Reset(0);
	mBuildBuckets.Reset(0); 
	mBuildObject.SetNum(renderInfos.Num());

	if (renderInfos.IsEmpty()) return;

	// AABB 및 inside, outside 나누기
	createRootNode(renderInfos);
	insertAllObjects(renderInfos);
}
