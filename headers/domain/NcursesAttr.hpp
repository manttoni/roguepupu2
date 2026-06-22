#pragma once

#include <string>
#include <ncurses.h>
#include <regex>

struct NcursesAttr
{
	chtype attr;

	std::string markup() const
	{
		switch (attr)
		{
			case A_DIM:
				return "[A_DIM]";
			case A_BOLD:
				return "[A_BOLD]";
			case A_REVERSE:
				return "[A_REVERSE]";
			case A_BLINK:
				return "[A_BLINK]";
			default:
				return "[A_NORMAL]";
		}
	}

	inline static bool is_markup(const std::string& str, const size_t idx)
	{
		if (idx >= str.size() || str[idx] != '[')
			return false;

		const auto close = str.find(']', idx);
		if (close == std::string::npos)
			return false;

		const auto markup = str.substr(idx, close - idx + 1);
		if (markup == "[reset]")
			return true;
		std::regex regex(R"(\[(A_[A-Z_]*)\])");
		std::smatch match;
		if (!std::regex_match(markup, match, regex))
			return false;

		const std::vector<std::string> handled_attrs = {"A_BOLD", "A_DIM", "A_REVERSE", "A_NORMAL", "A_BLINK"};
		auto it = std::find(handled_attrs.begin(), handled_attrs.end(), match[1]);
		return it != handled_attrs.end();
	}

	inline static chtype from_markup(const std::string& str, const size_t idx)
	{
		assert(is_markup(str, idx));
		const auto close = str.find(']', idx);
		const auto markup = str.substr(idx + 1, close - idx - 1);
		if (markup == "A_DIM") return A_DIM;
		if (markup == "A_BOLD") return A_BOLD;
		if (markup == "A_REVERSE") return A_REVERSE;
		if (markup == "A_BLINK") return A_BLINK;
		return A_NORMAL;
	}

	bool operator==(const NcursesAttr& other) const = default;
	bool operator==(const chtype attr) const { return attr == this->attr; }
	bool operator!=(const NcursesAttr& other) const = default;
	bool operator!=(const chtype attr) const { return attr != this->attr; }
};
