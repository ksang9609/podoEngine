#pragma once

#include "FileManager.h"
#include "Console.h"

#include <sstream>

class IniParser
{
public:
	float LoadSetting(const std::string& filename, FString Section, FString iniKey)
	{
		FFileManager iniMan = FFileManager();
		FString iniSetting = iniMan.ReadFileToString(filename);
		if (iniSetting.Len() == 0)
		{
			UE_LOG("No File");
			return 0.0f;
		}
		else
		{
			// FString Line Parsing
			int32 startPos = 0;
			FString CurrentSection;
			FString Key;
			FString Value;
			while (startPos < iniSetting.Len())
			{
				int32 endPos = iniSetting.Find(std::string_view("\n"), startPos);
				int32 lineLen = (endPos != -1) ? (endPos - startPos) : (iniSetting.Len() - startPos);

				FString line = iniSetting.Mid(startPos, lineLen);
				line.RemoveFromEnd(std::string_view("\r"));

				startPos = (endPos != -1) ? (endPos + 1) : iniSetting.Len();

				if (line.Len() == 0) continue;

				if (line.StartsWith(std::string_view("[")) && line.EndsWith(std::string_view("]")))
				{
					CurrentSection = line.Mid(line.Find(std::string_view("["))+1, line.Len() - 2);
				}
				else
				{
					int32 equalPos = line.Find(std::string_view("="));
					if (equalPos == -1)
						continue;
					Key = line.Mid(0, equalPos);
					Value = line.Mid(equalPos + 1, line.Len() - equalPos - 1);

					if (Section==CurrentSection && Key == iniKey)
					{
						return Value.ToFloat();
					}
				}

				
			}
		}
		return 0.0f;
	}
};
