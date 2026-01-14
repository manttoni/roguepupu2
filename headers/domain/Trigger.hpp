#pragma once

#include "Effect.hpp"
#include "Target.hpp"
#include "Conditions.hpp"
#include "Event.hpp"

struct Trigger
{
	// Check conditions when this type of event happens to the owner of this Trigger
	// Examples:
	// Type = gather, conditions = none, effect = destroy entity, target = self
	// When owned by a mushroom, it will get destoyed when something gathers from it
	// Type = gather, conditions = none, effect = gain xp, target = self
	// When owned by player, gain xp when gathering from anything
	// Type = EnterCell, conditions = weight > x, effect = create_entity, spike, target = cell
	// When something heavier than x enters cell of owner of this trigger, create a spike on the cell
	Event::Type type = Event::Type::None;
	Conditions conditions{};	// If these conditions are true
	Effect effect{};			// This effect will happen
	Target target{};			// To this target
};
