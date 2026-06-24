#pragma once

#include <string>
#include <iostream>
#include "domain/Actor.hpp"
#include "domain/Target.hpp"
#include "external/entt/fwd.hpp"
#include "domain/Damage.hpp"
#include "utils/Utils.hpp"


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
		AttackHit,
		AttackMiss,
		BecomeHostile,
	};

	Event& operator=(const Event& other) = default;

	std::string message = ""; // A general description will be formed in event handler
	Actor actor{};		// who does?
	Type type = Type::None;	// what?
	Target target{};	// to who?
	entt::entity weapon = entt::null; // with what?
	Damage::Roll damage_roll{};
	int advantage = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
	// Description
	os << Utils::without_markups(e.message) << std::endl;

	// Actor and target info
	if (e.actor.entity != entt::null)
		os << "Actor id: " << static_cast<uint32_t>(e.actor.entity);
	if (e.target.entity != entt::null)
		os << "\tTarget id: " << static_cast<uint32_t>(e.target.entity);
	os << std::endl;

	if (e.actor.position.is_valid())
		os << "Actor pos: " << e.actor.position;
	if (e.target.position.is_valid())
		os << "\tTarget pos: " << e.target.position;
	os << std::endl;

	os << "Event type id: " << static_cast<int>(e.type) << std::endl;

	if (e.type == Event::Type::Attack)
	{
		if (e.weapon != entt::null)
			os << "Weapon id: " << static_cast<uint32_t>(e.weapon);
		else
			os << "Weapon id: null/unarmed";

		if (e.advantage < 0)
			os << "\tDisadvantage: " << e.advantage;
		else if (e.advantage > 0)
			os << "\tAdvantage: " << e.advantage;
	}

	if (e.type == Event::Type::TakeDamage)
	{
		if (e.damage_roll.is_rolled)
			os << "\tDamage roll: " << e.damage_roll;
	}

	return os;
}
