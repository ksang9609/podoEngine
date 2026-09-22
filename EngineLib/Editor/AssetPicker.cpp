#include "AssetPicker.h"

#include "Core/AssetManager.h"
#include "ThirdParty/ImGui/imgui.h"

bool FAssetPicker::DrawStaticMeshPicker(const FAssetManager& assetManager, FName& outSelectedAssetKey)
{
	const FString previewName = outSelectedAssetKey.ToString();

	if (!ImGui::BeginCombo("Static Mesh", previewName.CStr())) return false;

	const bool bChanged = drawStaticMeshList(assetManager, outSelectedAssetKey);

	ImGui::EndCombo();
	return bChanged;
}

bool FAssetPicker::DrawMaterialPicker(const FAssetManager& assetManager, FName& outSelectedAssetKey, uint32 materialSlotNumber)
{
	const FString previewName = outSelectedAssetKey.ToString();
	const FString materialSlotLabel = FString(std::format("Material Slot {}", materialSlotNumber));

	if (!ImGui::BeginCombo(materialSlotLabel.CStr(), previewName.CStr())) return false;

	const bool bChanged = drawMaterialList(assetManager, outSelectedAssetKey);

	ImGui::EndCombo();
	return bChanged;
}

bool FAssetPicker::drawStaticMeshList(const FAssetManager& assetManager, FName& outSelectedAssetKey)
{
	bool bChanged = drawNoneOption(outSelectedAssetKey);

	const TArray<FName> staticMeshAssetKeys = assetManager.GetAllStaticMeshAssetKeys();

	for (const FName& assetKey : staticMeshAssetKeys)
	{
		const FString assetName = assetKey.ToString();
		const bool bSelected = assetKey == outSelectedAssetKey;

		if (ImGui::Selectable(assetName.CStr(), bSelected))
		{
			outSelectedAssetKey = assetKey;
			bChanged = true;
		}

		if (bSelected) ImGui::SetItemDefaultFocus();
	}

	return bChanged;
}

bool FAssetPicker::drawMaterialList(const FAssetManager& assetManager, FName& outSelectedAssetKey)
{
	bool bChanged = drawNoneOption(outSelectedAssetKey);
	const TArray<FName> materialAssetKeys = assetManager.GetAllMaterialAssetKeys();
	for (const FName& assetKey : materialAssetKeys)
	{
		const FString assetName = assetKey.ToString();
		const bool bSelected = assetKey == outSelectedAssetKey;
		if (ImGui::Selectable(assetName.CStr(), bSelected))
		{
			outSelectedAssetKey = assetKey;
			bChanged = true;
		}
		if (bSelected) ImGui::SetItemDefaultFocus();
	}
	return bChanged;
}

bool FAssetPicker::drawNoneOption(FName& outSelectedAssetKey)
{
	const bool bSelected = outSelectedAssetKey == FName();

	if (!ImGui::Selectable("None", bSelected)) return false;

	outSelectedAssetKey = FName();
	return true;
}
