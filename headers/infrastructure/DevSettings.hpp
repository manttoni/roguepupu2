#pragma once

struct DevSettings
{
	enum class Type
	{
		GodMode,
		ShowDebug,
	};
	struct Setting
	{
		std::string label;
		std::variant<bool> value;
	};
	std::map<Type, Setting> settings = {
		{ Type::GodMode, { "God Mode", false } },
		{ Type::ShowDebug, { "Show Debug", false } }
	};
};

