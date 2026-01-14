#pragma once

#include <string>
#include <optional>
#include "Target.hpp"
#include "Conditions.hpp"

struct Intent
{
	enum class Type
	{
		None,
		ExamineCell,
		OpenInventory,
		ShowPlayer,
		Move,
		Attack,
		Unlock,
		Open,
		DoNothing,
		UseAbility,
		Hide,
		Flee,
		Gather,
	};

	Type type = Type::None;
	std::optional<std::string> ability_id;
	std::optional<Target> target;
	std::optional<Conditions> conditions;
	//entt::entity actor = entt::null;
};
