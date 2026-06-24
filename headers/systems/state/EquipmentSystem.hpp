#pragma once

#include "external/entt/fwd.hpp"
#include "components/Components.hpp"

struct EquipmentSlotsUsed;
enum class EquipmentSlot;
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
	bool has_free_slots(const entt::registry& registry, const entt::entity entity, const EquipmentSlotsUsed& equipment_slot);
	bool equip_in_free_slots(entt::registry& registry, const entt::entity entity, const entt::entity item);
	entt::entity get_equipment_at(const entt::registry& registry, const entt::entity entity, const EquipmentSlot slot);
	bool melee_weapon_equipped(const entt::registry& registry, const entt::entity entity);
	bool ranged_weapon_equipped(const entt::registry& registry, const entt::entity entity);

	template<typename T>
		bool equipment_has_component(const entt::registry& registry, const entt::entity entity)
		{
			const auto equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
			for (const auto [slot, item] : equipped_items)
				if (registry.all_of<T>(item))
					return true;
			return false;
		}

	/*template<typename T>
		entt::entity get_equipped_item(const entt::registry& registry, const entt::entity entity)
		{
			const auto equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
			for (const auto [slots, item] : equipped_items)
				if (registry.all_of<T>(item))
					return item;
			return entt::null;
		}*/

	template<typename T>
		std::vector<entt::entity> get_equipped_items(const entt::registry& registry, const entt::entity entity)
		{
			const auto equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
			std::vector<entt::entity> items;
			for (const auto [slots, item] : equipped_items)
				if (registry.all_of<T>(item))
					items.push_back(item);
			return items;
		}
	std::vector<entt::entity> get_all_equipped_weapons(const entt::registry& registry, const entt::entity entity);
};
