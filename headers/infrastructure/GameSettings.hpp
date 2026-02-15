#pragma once

#include <map>
#include <string>

struct GameSettings
{
	enum class Type
	{
		ShowStatus, // show hp, mp and sp
		LogLength,	// how many lines of log to render
	};
	struct Setting
	{
		std::string label;
		std::variant<bool, int> value;
	};
	std::map<Type, Setting> settings = {
		{ Type::ShowStatus, { "Show status", true } },
		{ Type::LogLength, { "Log length", std::numeric_limits<int>::max() } }
	};
};
