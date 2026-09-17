#include "EditorViewportManager.h"

uint8_t FEditorViewportManager::addViewport()
{
	const uint8_t viewportId = allocateViewportId();
	if (viewportId <= 0)
	{
		return 0;
	}
	auto newViewport = std::make_unique<FViewportEntry>(viewportId, mSharedSettings);
	Viewports.Add(std::move(newViewport));

	return viewportId;
}
uint8_t FEditorViewportManager::allocateViewportId() const
{
	if (Viewports.Num() >= maxId)
	{
		return 0;
	}
	for (uint8_t id = 1; id <= maxId; id++)
	{
		if (findViewport(id) == nullptr)
		{
			return id;
		}
	}
	return 0;
}
bool FEditorViewportManager::removeViewport(uint8_t viewportId)
{
	if (Viewports.Num() <= 1)
	{
		return false;
	}
	const int8_t index = findViewportIndex(viewportId);
	if (index < 0)
	{
		return false;
	}
	Viewports.RemoveAt(static_cast<uint32_t>(index), 1);
	return true;
}
FViewportEntry* FEditorViewportManager::findViewport(uint8_t viewportId)
{
	const int8_t index = findViewportIndex(viewportId);
	if (index < 0)
	{
		return nullptr;
	}
	return Viewports[index].get();
}
const FViewportEntry* FEditorViewportManager::findViewport(uint8_t viewportId) const
{
	const int8_t index = findViewportIndex(viewportId);
	if (index < 0)
	{
		return nullptr;
	}
	return Viewports[index].get();
}
int8_t FEditorViewportManager::findViewportIndex(uint8_t viewportId) const
{
	if (viewportId == 0)
	{
		return -1;
	}
	for (uint8_t index = 0; index < Viewports.Num(); index++)
	{
		const FViewportEntry& entry = Viewports[index];
		if (entry && entry.Id == viewportId)
		{
			return index;
		}
	}
	return -1;
}
void FEditorViewportManager::updateViewports(float deltaTime, FSceneManager& sceneManager)
{

}
void FEditorViewportManager::renderViewports(FGraphicsManager& graphicsManager, FSceneManager& sceneManager)
{

}
void FEditorViewportManager::setCameraSpeed(float speed)
{
	mSharedSettings.cameraSpeed = speed;
}
void FEditorViewportManager::setSnapPreset(uint8_t presetIndex)
{
	mSharedSettings.snapPresetIndex = presetIndex;
}
