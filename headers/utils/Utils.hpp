#pragma once

#include <cctype>           // for toupper
#include <string>           // for basic_string, string, operator+, operator<<

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
}

