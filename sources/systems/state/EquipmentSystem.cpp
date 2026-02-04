#include <stddef.h>
#include <string>                       // for operator==, basic_string, ope...
#include <algorithm>
#include <vector>

#include "external/entt/entt.hpp"
#include "components/Components.hpp"               // for Equipment, Weapon, Armor (ptr...
#include "systems/state/EquipmentSystem.hpp"  // for Slot, equip, equip_armor, equ...
#include "utils/ECS.hpp"
#include "domain/Actor.hpp"
#include "domain/Effect.hpp"
#include "domain/Event.hpp"
#include "domain/Target.hpp"
#include "external/entt/entity/fwd.hpp"

namespace EquipmentSystem
{
	void equip_or_unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (is_equipped(registry, entity, item))
			unequip(registry, entity, item);
		else
			equip(registry, entity, item);
	}

	/* Equip item and unequip everything necessary to make space */
	void equip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		const auto slot = registry.get<Equipment>(item).slot;
		if (slot == Equipment::Slot::TwoHanded)
		{	// find and unequip all handheld equipment to make space
			for (const auto equipped_item : registry.get<EquipmentSlots>(entity).equipped_items)
			{
				const auto equipped_in = registry.get<Equipment>(equipped_item).slot;
				if (equipped_in == Equipment::Slot::OneHanded || equipped_in == Equipment::Slot::TwoHanded)
					unequip(registry, entity, equipped_item);
			}
			registry.get<EquipmentSlots>(entity).equipped_items.push_back(item);
		}
		else if (slot == Equipment::Slot::OneHanded)
		{
			size_t n = 0;
			for (const auto equipped : registry.get<EquipmentSlots>(entity).equipped_items)
			{
				const auto equipped_in = registry.get<Equipment>(equipped).slot;
				if (equipped_in == Equipment::Slot::OneHanded)
				{
					if (n == 0)
						n++;
					else
					{
						unequip(registry, entity, equipped);
						break;
					}
				}
				else if (equipped_in == Equipment::Slot::TwoHanded)
				{
					unequip(registry, entity, equipped);
					break;
				}
			}
			registry.get<EquipmentSlots>(entity).equipped_items.push_back(item);
		}
		Event equip_event;
		equip_event.actor.entity = entity;
		equip_event.effect.type = Effect::Type::Equip;
		equip_event.target.entity = item;
		ECS::queue_event(registry, equip_event);
	}

	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		auto& equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
		auto it = std::find(equipped_items.begin(), equipped_items.end(), item);
		if (it != equipped_items.end())
			equipped_items.erase(it);

		// Queue event: log a message, and it's a place to re-evaluate equipment bonuses
		Event unequip_event;
		unequip_event.actor.entity = entity;
		unequip_event.effect.type = Effect::Type::Unequip;
		unequip_event.target.entity = item;
		ECS::queue_event(registry, unequip_event);
	}

	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (entity == entt::null)
			return false;
		for (const auto equipped : registry.get<EquipmentSlots>(entity).equipped_items)
		{
			if (equipped == item)
				return true;
		}
		return false;
	}

	bool is_dual_wielding(const entt::registry& registry, const entt::entity entity)
	{
		size_t weapons = 0;
		for (const auto equipped : registry.get<EquipmentSlots>(entity).equipped_items)
		{
			if (registry.get<Category>(equipped).category == "weapons")
				weapons++;
		}
		return weapons == 2;
	}
};
