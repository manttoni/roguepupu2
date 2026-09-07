#pragma once
#include "ncurses/Attribute.hpp"
#include <cctype>           // for toupper
#include <codecvt>
#include <cwchar>
#include <locale>
#include <regex>
#include <stdexcept>
#include <string>           // for basic_string, string, operator+, operator<<
#include <string_view>
#include "ncurses/Color.hpp"

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

	// Remove {color} and [ncurses] attribute markups
	inline std::string without_markups(const std::string& str)
	{
		std::string without = "";
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (str[i] == '{' && Ncurses::Color::is_markup(str, i))
			{
				i = str.find('}', i);
				continue;
			}
			if (str[i] == '[' && Ncurses::Attribute::is_markup(str, i))
			{
				i = str.find(']', i);
				continue;
			}
			without += str[i];
		}
		return without;
	}

}

