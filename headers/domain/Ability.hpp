#pragma once

#include <string>
#include "Effect.hpp"
#include "Target.hpp"

struct Ability
{
	enum class Type
	{
		None,
		Spell,
		Innate,
	};

	Type type = Type::None;
	std::string id = "";
	size_t cooldown = 0;
	size_t last_used = 0;
	Effect effect{};
	Target target{};
};
