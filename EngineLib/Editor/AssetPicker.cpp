#include "AssetPicker.h"

#include "Core/AssetManager.h"
#include "ThirdParty/ImGui/imgui.h"

bool FAssetPicker::DrawStaticMeshPicker(const FAssetManager& assetManager, FName& selectedAssetKey)
{
	const FString previewName = selectedAssetKey.ToString();

	if (!ImGui::BeginCombo("Static Mesh", previewName.CStr())) return false;

	const bool bChanged = drawStaticMeshList(assetManager, selectedAssetKey);

	ImGui::EndCombo();
	return bChanged;
}

bool FAssetPicker::drawStaticMeshList(const FAssetManager& assetManager, FName& selectedAssetKey)
{
	bool bChanged = drawNoneOption(selectedAssetKey);

	const TArray<FName> staticMeshAssetKeys = assetManager.GetAllStaticMeshAssetKeys();

	for (const FName& assetKey : staticMeshAssetKeys)
	{
		const FString assetName = assetKey.ToString();
		const bool bSelected = assetKey == selectedAssetKey;

		if (ImGui::Selectable(assetName.CStr(), bSelected))
		{
			selectedAssetKey = assetKey;
			bChanged = true;
		}

		if (bSelected) ImGui::SetItemDefaultFocus();
	}

	return bChanged;
}

bool FAssetPicker::drawNoneOption(FName& selectedAssetKey)
{
	const bool bSelected = selectedAssetKey == FName();

	if (!ImGui::Selectable("None", bSelected)) return false;

	selectedAssetKey = FName();
	return true;
}
