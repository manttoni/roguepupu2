#include <stddef.h>
#include <string>                       // for operator==, basic_string, ope...
#include <algorithm>
#include <vector>

#include "external/entt/entt.hpp"
#include "components/Components.hpp"               // for Equipment, Weapon, Armor (ptr...
#include "systems/state/EquipmentSystem.hpp"  // for Slot, equip, equip_armor, equ...
#include "systems/state/InventorySystem.hpp"
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

	void equip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!registry.all_of<Equipment>(item) ||
				!registry.all_of<EquipmentSlots>(entity) ||
				is_equipped(registry, entity, item) ||
				!InventorySystem::has_item(registry, entity, item))
			return;
		const auto& equipment = registry.get<Equipment>(item);
		auto& equipment_slots = registry.get<EquipmentSlots>(entity);
		auto& equipped_items = equipment_slots.equipped_items;
		assert(equipment.use_one.has_value() != equipment.use_all.has_value());
		if (equipment.use_one.has_value())
		{
			// Equip item in first free slot it can be equipped in
			for (const auto slot : *equipment.use_one)
			{
				if (equipped_items[slot] == entt::null)
				{
					equipped_items[slot] = item;
					break;
				}
			}
			// If no such slot was available, force equip in a slot
			if (!is_equipped(registry, entity, item))
			{
				unequip(registry, entity, equipped_items[equipment.use_one->front()]);
				equipped_items[equipment.use_one->front()] = item;
			}
		}
		else if (equipment.use_all.has_value())
		{
			// Unequip everything in the slots and set item there
			for (const auto slot : *equipment.use_all)
			{
				unequip(registry, entity, equipped_items[slot]);
				equipped_items[slot] = item;
			}
		}
		assert(is_equipped(registry, entity, item));

		Event event = {.type = Event::Type::Equip};
		event.actor.entity = entity;
		event.target.entity = item;
		ECS::queue_event(registry, event);
	}

	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!registry.all_of<Equipment>(item) ||
				!registry.all_of<EquipmentSlots>(entity) ||
				!is_equipped(registry, entity, item))
			return;
		auto& equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
		for (auto& [slot, equipped_item] : equipped_items)
		{
			if (item == equipped_item)
				equipped_items[slot] = entt::null;
		}

		Event event = {.type = Event::Type::Unequip};
		event.actor.entity = entity;
		event.target.entity = item;
		ECS::queue_event(registry, event);
	}

	void swap_loadout(entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<EquipmentSlots>(entity))
			return;
		auto& equipment_slots = registry.get<EquipmentSlots>(entity);
		auto& equipped_items = equipment_slots.equipped_items;
		auto& active_loadout = equipment_slots.loadouts[equipment_slots.active_loadout];

		// update current loadout
		active_loadout.main_hand = equipped_items[Equipment::Slot::MainHand];
		active_loadout.off_hand = equipped_items[Equipment::Slot::OffHand];

		unequip(registry, entity, active_loadout.main_hand);
		unequip(registry, entity, active_loadout.off_hand);

		equipment_slots.active_loadout ^= 1;
		auto& other_loadout = equipment_slots.loadouts[equipment_slots.active_loadout];
		equip(registry, entity, other_loadout.main_hand);
		equip(registry, entity, other_loadout.off_hand);
	}

	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!registry.all_of<Equipment>(item) ||
				!registry.all_of<EquipmentSlots>(entity))
			return false;
		const auto& equipment_slots = registry.get<EquipmentSlots>(entity);
		for (const auto& [slot, equipped_item] : equipment_slots.equipped_items)
		{
			if (item == equipped_item)
				return true;
		}

		return false;
	}

	bool shield_equipped(const entt::registry& registry, const entt::entity entity)
	{
		(void) registry; (void) entity;
		return false;
	}

	bool is_dual_wielding(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<EquipmentSlots>(entity))
			return false;
		const auto& equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
		if (!equipped_items.contains(Equipment::Slot::MainHand) || equipped_items.at(Equipment::Slot::MainHand) == entt::null)
			return false;
		if (!equipped_items.contains(Equipment::Slot::OffHand) || equipped_items.at(Equipment::Slot::OffHand) == entt::null)
			return false;
		return equipped_items.at(Equipment::Slot::MainHand) != equipped_items.at(Equipment::Slot::OffHand) &&
			!shield_equipped(registry, entity);
	}

	/* Return true if entity can equip equipment without unequipping something first
	 * */
	bool has_free_slots(const entt::registry& registry, const entt::entity entity, const Equipment& equipment)
	{
		const auto& equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
		assert(equipment.use_all.has_value() != equipment.use_one.has_value());
		if (equipment.use_all.has_value())
		{
			for (const auto& slot : *equipment.use_all)
			{
				if (equipped_items.at(slot) != entt::null)
					return false;
			}
			return true;
		}
		else if (equipment.use_one.has_value())
		{
			for (const auto& slot : *equipment.use_one)
			{
				if (equipped_items.at(slot) == entt::null)
					return true;
			}
			return false;
		}
		Error::fatal("No such item should exist");
		//return true; // This equipment doesnt need any slot
	}

	/* Like equip() but does not touch equipped items.
	 * return true if did equip it
	 * */
	bool equip_in_free_slots(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		for (size_t i = 0; i < 2; ++i)
		{
			if (has_free_slots(registry, entity, registry.get<Equipment>(item)))
			{
				equip(registry, entity, item);
				return true;
			}
			swap_loadout(registry, entity);
		}
		return false;
	}
};
