#pragma once

#include "external/entt/fwd.hpp"

struct Equipment;
namespace EquipmentSystem
{
	void equip_or_unequip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip_weapon(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip_armor(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void swap_loadout(entt::registry& registry, const entt::entity entity);
	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item);
	bool is_dual_wielding(const entt::registry& registry, const entt::entity entity);
	bool has_free_slots(const entt::registry& registry, const entt::entity entity, const Equipment& equipment);
	bool equip_in_free_slots(entt::registry& registry, const entt::entity entity, const entt::entity item);
};
