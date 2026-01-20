#pragma once

#include "domain/Damage.hpp"

/* Effect is a data structure describing what happens
 * It does not know to who it happens or by who
 * Some effects will trigger Triggers
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
		Damage,
		Heal,
		CreateEntity,
		DestroyEntity,
		Ignite,
		SelfDestruct,
		ReceiveItem,
	};
	Type type = Type::None;

	// Entity of CreateEntity effect. Maybe even DestroyEntity?
	std::string entity_id = "";

	// Affects everything within
	double radius = 0.0;	// 0.0 will be just one cell

	// f.e. ignite strength, healing strength
	double magnitude = 0.0;

	// When dealing direct damage
	Damage damage{};



};
