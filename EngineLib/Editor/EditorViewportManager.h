#pragma once
#include "../Core/Container/TArray.h"
#include "Viewport.h"
#include "ViewportLayout.h"

#include <memory>

class FEditorViewportClient;
class FGraphicsManager;
class FSceneManager;

class FEditorViewportManager
{
private :
	FViewportSharedSettings mSharedSettings;
	TArray<std::unique_ptr<FViewport>> Viewports;

	EViewportLayoutMode layoutMode = EViewportLayoutMode::SinglePane;
	uint8 activeViewportId = invalidViewportId;

	std::unique_ptr<SWindow> layoutRoot;
	SSplitter* draggingSplitter = nullptr;
public :
	FEditorViewportManager() = default;
	~FEditorViewportManager() = default;

	bool Initialize();

	FViewport* findViewport(uint8 viewportId);
	const FViewport* findViewport(uint8 viewportId) const;

	FViewport* getViewportAt(uint8 index);
	const FViewport* getViewportAt(uint8 index) const;

	FViewport* getActiveViewport();
	const FViewport* getActiveViewport() const;

	bool setActiveViewport(uint8 viewportId);

	uint8 getViewportCount() const;
	EViewportLayoutMode getLayoutMode() const;

	uint8 addViewport();
	bool removeViewport(uint8 viewportId);

	void arrangeLayout(const FRect& hostRect);

	bool beginSplitterDrag(const FPoint& point);
	void updateSplitterDrag(const FPoint& point);
	void endSplitterDrag();

	inline const FViewportSharedSettings& getSharedSettings() const { return mSharedSettings;  }

	void setCameraSpeed(float speed);
	void setSnapPreset(uint8 presetIndex);

private:
	uint8 allocateViewportId() const;
	int8 findViewportIndex(uint8 viewportId) const;
	void reindexViewports();

	void rebuildLayout();

	std::unique_ptr<SWindow> makeViewportPanel(uint8 index);

	std::unique_ptr<SWindow> makeSingleLayout();
	std::unique_ptr<SWindow> makeTwoPaneLayout();
	std::unique_ptr<SWindow> makeThreePaneLayout();
	std::unique_ptr<SWindow> makeFourPaneLayout();
};
