#pragma once

#include <string>
#include "entt.hpp"

namespace EquipmentSystem
{
	void equip_or_unequip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip_weapon(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip_armor(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item);
	bool is_dual_wielding(const entt::registry& registry, const entt::entity entity);
};
