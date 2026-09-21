#pragma once

#include "Core/Core.h"
#include "../Core/Math/MathUtility.h"
#include <fstream>

struct FEditorSetting
{
	float CameraSensitivity = 0.1f;
	float GridSpacing = 0.5f;

	uint8 viewportCount = 1;

	float twoPaneVertical = 0.5f;

	float threePaneVertical = 0.5f;
	float threePaneRightHorizontal = 0.5;

	float fourPaneHorizontal = 0.5f;
	float fourPaneVertical = 0.5f;

	uint8 maxViewportCount = 4;

	void Load(const std::string& FilePath = "Config/editor.ini")
	{
		std::ifstream File(FilePath);

		if (!File.is_open())
		{
			throw std::runtime_error("Load failed");
		}

		std::string Key;

		while (File >> Key)
		{
			if (Key == "CameraSensitivity")
			{
				File >> CameraSensitivity;
			}
			if (Key == "GridSpacing")
			{
				File >> GridSpacing;
			}
			if (Key == "ViewportCount")
			{
				int32 loadedViewportCount = 1;
				File >> loadedViewportCount;
				viewportCount = static_cast<uint8>(FMath::Clamp(loadedViewportCount,1,static_cast<int32>(maxViewportCount)));
			}
			if (Key == "TwoPaneVertical")
			{
				File >> twoPaneVertical;
			}
			if (Key == "ThreePaneVertical")
			{
				File >> threePaneVertical;
			}
			if (Key == "ThreePaneRightHorizontal")
			{
				File >> threePaneRightHorizontal;
			}
			if (Key == "FourPaneHorizontal")
			{
				File >> fourPaneHorizontal;
			}
			if (Key == "FourPaneVertical")
			{
				File >> fourPaneVertical;
			}
		}
		
	}

	void Save(const std::string& FilePath = "Config/editor.ini") const
	{
		std::ofstream File(FilePath);

		if (!File.is_open())
		{
			throw std::runtime_error("Save failed");
		}

		File << "CameraSensitivity " << CameraSensitivity << '\n';
		File << "GridSpacing " << GridSpacing << '\n';
		File << "ViewportCount " << static_cast<int32>(viewportCount) << '\n';
		File << "TwoPaneVertical " << twoPaneVertical << '\n';
		File << "ThreePaneVertical " << threePaneVertical << '\n';
		File << "ThreePaneRightHorizontal " << threePaneRightHorizontal << '\n';
		File << "FourPaneHorizontal " << fourPaneHorizontal << '\n';
		File << "FourPaneVertical " << fourPaneVertical << '\n';
	}
};
