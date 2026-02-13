#pragma once

#include <optional>
#include "domain/Damage.hpp"

/* The future of this is uncertain, maybe it will be for abilities or spells or something
 * */

struct Effect
{
	enum class Type
	{
		None,
		Unequip,
		Equip,
		Drop,
		Death,
		Spawn,
		Gather,
		Move,
		TakeDamage,
		Heal,
		CreateEntity,
		DestroyEntity,
		Ignite,
		SelfDestruct,
		ReceiveItem,
		Attack,
	};
	Type type = Type::None;

	// Entity of CreateEntity effect. Maybe even DestroyEntity?
	std::string entity_id = "";

	// Affects everything within
	double radius = 0.0;	// 0.0 will be just one cell

	// f.e. ignite strength, healing strength
	double magnitude = 0.0;

	// When dealing direct damage
	std::optional<Damage::Spec> damage = std::nullopt;
};
