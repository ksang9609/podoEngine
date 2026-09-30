#include "FOctree.h"

void FOctree::addSubtreeAll(uint32 nodeIndex, const FVector& cameraPos, TArray<uint32> & outVisible) const
{
	// Inside 확정된 가지만 모아서 재귀해주는 함수
	const FOctreeNode &node = mNodes[nodeIndex];

	for (int i = 0; i < node.ObjectCount; i++) {
		const uint32 objectIndex = mInsideIndices[node.ObjectStart + i];
		if (mStaleFlags[objectIndex]) continue;   
		outVisible.Add(objectIndex);
	}

	if (node.ChildStart == 0) return;


	const uint8 nearestOctant = (cameraPos.x >= node.Center.x ? 1 : 0)
		+ (cameraPos.y >= node.Center.y ? 2 : 0)
		+ (cameraPos.z >= node.Center.z ? 4 : 0);

	for (uint32 i = 0; i < 8; i++) {
		uint8 octant = nearestOctant ^ i;	// 카메라와 가까운순 탐색
		addSubtreeAll(node.ChildStart + octant, cameraPos, outVisible);
	}
}

void FOctree::cullNode(uint32 nodeIndex, const FVector & cameraPos, const FFrustum& frustum, TArray<uint32>& outInside, TArray<uint32>& outIntersect) const
{
	// 해당 노드가 Outside/Inside/Intersect 상태인지 판별해서 outInside/outIntersect 에 담아줌
	const FOctreeNode &node = mNodes[nodeIndex];

	// FBoundingBox 로 일단 state 반환하기 
	const float looseHalf = node.HalfSize * 2.0f;
	FBoundingBox nodeBounds;
	nodeBounds.min = node.Center - FVector(looseHalf);
	nodeBounds.max = node.Center + FVector(looseHalf);
	EContainment state = frustum.Contains(nodeBounds);

	switch (state)
	{
		case EContainment::Outside:
			return;
		case EContainment::Inside:
			addSubtreeAll(nodeIndex, cameraPos, outInside);
			return;
		case EContainment::Intersect:
		{
			for (uint32 i = 0; i < node.ObjectCount; i++)
			{
				const uint32 objectIndex = mInsideIndices[node.ObjectStart + i];
				if (mStaleFlags[objectIndex]) continue;
				outIntersect.Add(objectIndex);
			}

			if (node.ChildStart == 0) return;

			const uint8 nearestOctant = (cameraPos.x >= node.Center.x ? 1 : 0)
				+ (cameraPos.y >= node.Center.y ? 2 : 0)
				+ (cameraPos.z >= node.Center.z ? 4 : 0);

			for (uint32 i = 0; i < 8; i++) {
				uint8 octant = nearestOctant ^ i;	// 카메라와 가까운순 탐색
				cullNode(node.ChildStart + octant, cameraPos, frustum, outInside, outIntersect);
			}
			return;
		}
	}
}

uint32 FOctree::GetMaxNodeObjectCount() const
{
	uint32 maxCount = 0;

	for (const FOctreeNode& node : mNodes)
	{
		if (node.ObjectCount > maxCount)
		{
			maxCount = node.ObjectCount;
		}
	}

	return maxCount;
}

void FOctree::FrustumCull(const FFrustum& frustum, const FVector & cameraPos, TArray<uint32>& outInside, TArray<uint32>& outIntersect) const
{
	outInside.Reset(0);
	outIntersect.Reset(0);

	if (mNodes.Num() == 0) return;

	cullNode(0, cameraPos, frustum, outInside, outIntersect);
	for (auto i : mOutsideObjects) {
		outIntersect.Add(i);
	}
}

void FOctree::createRootNode(const TArray<const FRenderInfo*>& renderInfos)
{
	if (mbHasRootBox)
	{
		FOctreeNode cachedRoot;
		cachedRoot.Center = mRootCenter;
		cachedRoot.HalfSize = mRootHalfSize;
		cachedRoot.ChildStart = 0;
		cachedRoot.ObjectStart = 0;
		cachedRoot.ObjectCount = 0;

		mNodes.Add(cachedRoot);
		mBuildBuckets.Add(TArray<uint32>());

		int32 fitCount = 0;
		for (const FRenderInfo* renderInfo : renderInfos)
		{
			if (!outsideRoot(renderInfo->WorldBounds))
			{
				fitCount++;
			}
		}

		if (fitCount * 10 >= renderInfos.Num() * 9)
		{
			return;
		}

		mNodes.Reset(0);
		mBuildBuckets.Reset(0);
	}




	FVector sceneMin = FLT_MAX;
	FVector sceneMax = -FLT_MAX;

	for (const FRenderInfo* renderInfo : renderInfos) {
		sceneMin.x = (std::min)(sceneMin.x, renderInfo->WorldBounds.min.x);
		sceneMin.y = (std::min)(sceneMin.y, renderInfo->WorldBounds.min.y);
		sceneMin.z = (std::min)(sceneMin.z, renderInfo->WorldBounds.min.z);
		sceneMax.x = (std::max)(sceneMax.x, renderInfo->WorldBounds.max.x);
		sceneMax.y = (std::max)(sceneMax.y, renderInfo->WorldBounds.max.y);
		sceneMax.z = (std::max)(sceneMax.z, renderInfo->WorldBounds.max.z);
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
	mRootCenter = rootNode.Center;
	mRootHalfSize = rootNode.HalfSize;
	mbHasRootBox = true;
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
			mStaleFlags[i] = true;
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

	mBuildBuckets.Reset(0);
}

void FOctree::Build(const TArray<const FRenderInfo*>& renderInfos)
{
	mNodes.Reset(0);
	mInsideIndices.Reset(0);
	mOutsideObjects.Reset(0);
	mBuildObject.Reset(0);
	mBuildBuckets.Reset(0);
	mStaleFlags.Reset(0);


	mBuildObject.SetNum(renderInfos.Num());
	mStaleFlags.SetNum(renderInfos.Num());


	if (renderInfos.IsEmpty()) return;

	// AABB 및 inside, outside 나누기
	createRootNode(renderInfos);
	insertAllObjects(renderInfos);
}

void FOctree::Raycast(const FVector& origin, const FVector& direction, TArray<uint32>& outCandidates) const
{
	FVector invDir;
	invDir.x = 1.0f / direction.x;
	invDir.y = 1.0f / direction.y;
	invDir.z = 1.0f / direction.z;
	outCandidates.Reset(0);
	raycastNode(0, origin, invDir, outCandidates);	// mInsideObjects 알아서 재귀됨
	for (auto object : mOutsideObjects) {
		outCandidates.Add(object);
	}
}

void FOctree::MarkObjectMoved(uint32 objectIndex)
{
	if (objectIndex >= static_cast<uint32> (mStaleFlags.Num())) return;
	if (mStaleFlags[objectIndex]) return;

	mStaleFlags[objectIndex] = true;
	mOutsideObjects.Add(objectIndex);
}

bool FOctree::NeedsRebuild() const
{
	const int32 total = mStaleFlags.Num();
	if (total == 0) return false;

	return mOutsideObjects.Num() * 10 > total;
}

void FOctree::raycastNode(uint32 nodeIndex, const FVector& origin, const FVector& invDir, TArray<uint32>& outCandidates) const
{
	float outTEnter = 0.0f;
	if (!intersectLooseBounds(nodeIndex, origin, invDir, 1.0f, outTEnter)) return;

	const FOctreeNode& node = mNodes[nodeIndex];
	for (int32 i = 0; i < node.ObjectCount; i++) {
		const uint32 objectIndex = mInsideIndices[node.ObjectStart + i];
		if (mStaleFlags[objectIndex]) continue;
		outCandidates.Add(objectIndex);
	}

	if (node.ChildStart == 0) return;

	for (int num = 0; num < 8; num++) {
			raycastNode(node.ChildStart+num, origin, invDir, outCandidates);
	}
}

bool FOctree::intersectLooseBounds(uint32 nodeIndex, const FVector& origin, const FVector& invDir, float tMax, float& outTEnter) const
{
	FVector nodeCenter = mNodes[nodeIndex].Center;
	float nodeHalfSize = mNodes[nodeIndex].HalfSize;
	FVector looseHalfSize = nodeHalfSize * 2.0f;

	float tMin = 0.0f;

	for (int axis = 0; axis < 3; ++axis)
	{
		const float minValue = nodeCenter[axis] - looseHalfSize[axis];
		const float maxValue = nodeCenter[axis] + looseHalfSize[axis];

		float t1 = (minValue - origin[axis]) * invDir[axis];
		float t2 = (maxValue - origin[axis]) * invDir[axis];

		if (t1 > t2)
		{
			std::swap(t1, t2);
		}

		tMin = (std::max)(tMin, t1);
		tMax = (std::min)(tMax, t2);

		if (tMin > tMax)
		{
			return false;
		}
	}

	outTEnter = tMin;
	return true;
}
