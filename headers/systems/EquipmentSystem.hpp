#pragma once

#include <string>
#include "entt.hpp"

namespace EquipmentSystem
{
	enum class Slot
	{
		one_handed, // one_hand and two_hand are special cases,
		two_handed, // they can be in left and/or right hand
		left_hand,
		right_hand,
		body,
		gloves,
		helmet,
		boots,
		ring,
		amulet,
		cloak
	};
	Slot parse_slot(const std::string& str);
	void equip_or_unequip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip_weapon(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip_armor(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	bool is_equipped(const entt::registry& registry, const entt::entity& entity, const entt::entity& item);
};
