#pragma once

#include <string>
#include "domain/Actor.hpp"
#include "domain/Target.hpp"
#include "external/entt/fwd.hpp"
#include "domain/Damage.hpp"


/* Describes something that has happaned
 * "actor does something to target (with something)"
 * Can be logged as a message
 * "Rabdin crafts something"
 * "Rabdin ignites goblin"
 * "Rabdin moves"
 *
 * Actor and Target are the same thing with different type,
 * to force type/role
 *
 * This will be put in a queue when the event has happened.
 * EventSystem will review the event, log it in game log,
 * and call possible after effects, like OnEnterCell
 * */

struct Event
{
	enum class Type
	{
		None,
		Unequip,
		Equip,
		Drop,
		ReceiveItem,
		Death,
		Spawn,
		Destroy,
		Gather,
		Move,
		TakeDamage,
		Attack,
		BecomeHostile,
	};
	Actor actor{};		// who does?
	Type type = Type::None;	// what?
	Target target{};	// to who?
	entt::entity weapon = entt::null; // with what?
	Damage::Roll damage_roll{};
	int hit_quality = 0;
};
