#pragma once

#include "Effect.hpp"
#include "Actor.hpp"
#include "Conditions.hpp"

/* Triggers are owned by entities
 * When Trigger::Type happens to that entity,
 * effect will happen to target if conditions are true
 * Target is usually triggerer, but can be self.
 * */

struct Trigger
{
	enum class Type
	{
		None,
		Gather,		// triggers when owner gathers or is gathered from
		EnterCell,	// triggers when something enters the same cell as owner
		Destroy,	// triggers when owner is destroyed
		Create,		// triggers when owner is created
		Damage,		// triggers when owner is damaged
		Light,		// triggers when owner is illuminated by light
		Ignite,		// triggers when owner is ignited
	};
	Type type = Type::None;
	Conditions conditions{};
	Effect effect{};
};
