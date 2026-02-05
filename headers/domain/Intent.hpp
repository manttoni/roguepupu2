#pragma once

#include <string>
#include "domain/Conditions.hpp"
#include "domain/Actor.hpp"
#include "domain/Target.hpp"

/* NPC intentions come from AI component.
 * PC intentions will come from user input.
 *
 * Intent will not necessarily happen.
 * AI will evaluate Conditions to see if it "wants"
 * to actually do the action.
 * */

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
	Actor actor{};
	Target target{};
	std::string ability_id = "";

	bool operator==(const Intent& other) const = default;
	bool operator!=(const Intent& other) const = default;
};
