#pragma once

#include <string>
#include <optional>
#include "Color.hpp"

struct Effect
{
	enum class Type
	{
		None,
		CreateEntity,
		DestroyEntity,
		Transition,
		SetFGColor,
	};

	Type type = Type::None;
	std::optional<std::string> entity_id;
	std::optional<Color> fgcolor;
};
