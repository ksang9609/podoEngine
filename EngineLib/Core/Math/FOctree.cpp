#include "FOctree.h"

void FOctree::createRootNode(const TArray<FRenderInfo>& renderInfos)
{
	FVector sceneMin = FLT_MAX;
	FVector sceneMax = -FLT_MAX;

	for (const auto& renderInfo : renderInfos) {
		sceneMin.x = std::min(sceneMin.x, renderInfo.WorldBounds.min.x);
		sceneMin.y = std::min(sceneMin.y, renderInfo.WorldBounds.min.y);
		sceneMin.z = std::min(sceneMin.z, renderInfo.WorldBounds.min.z);
		sceneMax.x = std::max(sceneMax.x, renderInfo.WorldBounds.max.x);
		sceneMax.y = std::max(sceneMax.y, renderInfo.WorldBounds.max.y);
		sceneMax.z = std::max(sceneMax.z, renderInfo.WorldBounds.max.z);
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
}

void FOctree::insertAllObjects(const TArray<FRenderInfo>& renderInfos)
{
	for (int32 i = 0; i < renderInfos.Num(); i++) {
		if (outsideRoot(renderInfos[i].WorldBounds)) {
			mOutsideObjects.Add(i);
			continue;
		}
		mInsideIndices.Add(i);
	}

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

void FOctree::Build(const TArray<FRenderInfo>& renderInfos)
{
	mNodes.Reset(0);
	mInsideIndices.Reset(0);
	mOutsideObjects.Reset(0);

	if (renderInfos.IsEmpty()) return;

	createRootNode(renderInfos);
	insertAllObjects(renderInfos);
}
