#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

namespace PathEncoding
{
	inline std::filesystem::path FromUtf8(std::string_view value)
	{
		return std::filesystem::path(std::u8string(value.begin(), value.end()));
	}

	// Engine paths are UTF-8. Accept legacy Windows code-page paths at input boundaries.
	inline std::filesystem::path FromExternal(std::string_view value)
	{
		try
		{
			return FromUtf8(value);
		}
		catch (const std::system_error&)
		{
			return std::filesystem::path(value);
		}
	}

	inline std::string ToUtf8(const std::filesystem::path& value)
	{
		const auto utf8 = value.generic_u8string();
		return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
	}
}
