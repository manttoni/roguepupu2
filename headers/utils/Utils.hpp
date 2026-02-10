#pragma once

#include <cctype>           // for toupper
#include <string>           // for basic_string, string, operator+, operator<<
#include <regex>
#include <codecvt>
#include <locale>

namespace Utils
{
	inline std::string capitalize(const std::string& str)
	{
		std::string cap = str;
		cap[0] = std::toupper(cap[0]);
		size_t pos = 0;
		while ((pos = str.find(' ', pos)) != std::string::npos)
		{
			pos++;
			cap[pos] = std::toupper(cap[pos]);
		}
		return cap;
	}

	inline std::string conjugate_third(const std::string& str)
	{
		static const std::vector<std::string> es_endings = {"ch", "sh", "s", "x", "z", "o", "es"};
		static const std::string vocals = "aeiou";

		for (const auto& ending : es_endings)
		{
			if (str.ends_with(ending))
				return str + "es";
		}
		if (str.ends_with("y"))
		{
			const char second_last = str[str.size() - 2];
			if (vocals.find(second_last) == std::string::npos)
				return str.substr(0, str.size() - 1) + "ies";
		}
		return str + "s";
	}

	inline std::string to_utf8(const wchar_t w)
	{
		std::wstring ws(1, w);
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.to_bytes(ws);
	}
}

