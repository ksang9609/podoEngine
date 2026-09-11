#pragma once

#include "Core.h"
#include <fstream>

struct FEditorSetting
{
	float CameraSensitivity = 0.1f;

	void Load(const std::string& FilePath = "editor.ini")
	{
		std::ifstream File(FilePath);

		if (!File.is_open())
		{
			return;
		}

		std::string Key;

		while (File >> Key)
		{
			if (Key == "CameraSensitivity")
			{
				File >> CameraSensitivity;
			}
		}
	}

	void Save(const std::string& FilePath = "editor.ini") const
	{
		std::ofstream File(FilePath);

		if (!File.is_open())
		{
			return;
		}

		File << "CameraSensitivity " << CameraSensitivity << '\n';
	}
};
