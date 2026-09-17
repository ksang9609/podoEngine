#include "../Core/Container/TArray.h"

#include <memory>

class FEditorViewportClient;
class FGraphicsManager;
class FSceneManager;

struct FViewportSharedSettings
{
	inline static constexpr std::array<float, 4> snapPresets{ 0.01f, 0.1f, 1.0f, 5.0f };
	uint8_t snapPresetIndex = 1;
	float cameraSpeed = 5.0f;
	float GetSnapSize() const { return snapPresets[snapPresetIndex]; }
};

struct FViewportEntry
{
	FViewportEntry(uint8_t id, FViewportSharedSettings& sharedSettings)
		:Id(id), Client(std::make_unique<FEditorViewportClient>(sharedSettings))
	{};
	~FViewportEntry();

	uint8_t Id = 0;
	std::unique_ptr<FEditorViewportClient> Client = nullptr;
};

enum paneMode
{
	SinglePane,
	TwoPane,
	ThreePane,
	FourPane
};

class FEditorViewportManager
{
private :
	FViewportSharedSettings mSharedSettings;
	paneMode mPaneMode = SinglePane;
	TArray<std::unique_ptr<FViewportEntry>> Viewports;
	const uint8_t maxId = 4;
public :
	FEditorViewportManager() = default;
	~FEditorViewportManager() = default;

	uint8_t addViewport();
	uint8_t allocateViewportId() const;
	bool removeViewport(uint8_t viewportId);

	FViewportEntry* findViewport(uint8_t viewportId);
	const FViewportEntry* findViewport(uint8_t viewportId) const;

	int8_t findViewportIndex(uint8_t viewportId) const;

	void updateViewports(float deltaTime, FSceneManager& sceneManager);
	void renderViewports(FGraphicsManager& graphicsManager, FSceneManager& sceneManager);

	const FViewportSharedSettings& getSharedSettings() const { return mSharedSettings;  }
	void setCameraSpeed(float speed);
	void setSnapPreset(uint8_t presetIndex);
};
