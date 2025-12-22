#pragma once

#include <string>
#include "entt.hpp"
class Cell;

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
		Transition,
		Unlock,
		Open,
		DoNothing,
		UseAbility,
		Hide,
		Flee
	};

	Type type = Type::None;
	Cell* target_cell = nullptr;
	entt::entity target = entt::null;
	std::string ability_id = "";
};
