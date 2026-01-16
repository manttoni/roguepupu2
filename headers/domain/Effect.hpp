#pragma once

#include "Color.hpp"

/* Effect is a data structure describing what happens
 * It does not know to who it happens or by who
 * Some effects will trigger Triggers
 * */

struct Effect
{
	enum class Type
	{
		None,
		Gather,			// actor.entity gathers from target... hmm...
		Move,			// actor.entity moves to target.position
		Damage,			// target is damaged
		Heal,			// target is healed
		CreateEntity,	// target is created
		DestroyEntity,	// target is destroyed
		Transition,		// target.entity is transported to target.position
		Ignite,			// target (position and entity) is ignited
	};


	Type type = Type::None;

	// Affects everything within
	double radius = 0.0;	// 0.0 will be just one cell

	// f.e. ignite strength, healing strength
	double magnitude = 0.0;

	// When dealing direct damage
	Damage damage{};
};
