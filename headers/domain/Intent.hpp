#pragma once

#include <string>
#include "domain/Conditions.hpp"
#include "domain/Actor.hpp"

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
	std::string ability_id;	// id of ability to use, empty for none
	Conditions conditions;	// If these are false, don't do this action
	Actor target;			// While configuring the Intent, save target data here
};
