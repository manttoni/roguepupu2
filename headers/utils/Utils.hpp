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

	inline bool is_color_markup(const std::string& str, const size_t idx)
	{
		if (str[idx] != '{')
			return false;

		const auto close = str.find('}', idx);
		if (close == std::string::npos)
			return false;

		const auto markup = str.substr(idx, close - idx);
		if (markup == "{reset}")
			return true;
		std::regex regex(R"(\{\d+,\d+,\d+\})");
		std::smatch match;
		if (!std::regex_match(markup, match, regex))
			return false;

		const int r = std::stoi(match[1].str());
		const int g = std::stoi(match[2].str());
		const int b = std::stoi(match[3].str());

		if (r < 0 || r > 1000 ||
				g < 0 || g > 1000 ||
				b < 0 || b > 1000)
			return false;
		return true;
	}
	inline bool is_attr_markup(const std::string& str, const size_t idx)
	{
		if (str[idx] != '[')
			return false;

		const auto close = str.find(']', idx);
		if (close == std::string::npos)
			return false;

		const auto markup = str.substr(idx, close - idx);
		if (markup == "{reset}")
			return true;
		std::regex regex(R"(\[(A_[A-Z_]*)\])");
		std::smatch match;
		if (!std::regex_match(markup, match, regex))
			return false;

		const std::vector<std::string> handled_attrs = {"A_BOLD", "A_DIM"};
		auto it = std::find(handled_attrs.begin(), handled_attrs.end(), match[1]);
		return it != handled_attrs.end();

	}

	inline std::string to_utf8(const wchar_t w)
	{
		std::wstring ws(1, w);
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.to_bytes(ws);
	}
}

