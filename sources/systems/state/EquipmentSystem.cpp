#include <string>                       // for operator==, basic_string, ope...
#include "ECS.hpp"
#include "Components.hpp"               // for Equipment, Weapon, Armor (ptr...
#include "Utils.hpp"                    // for error
#include "entt.hpp"                     // for vector, entity, null_t, find
#include "systems/EquipmentSystem.hpp"  // for Slot, equip, equip_armor, equ...

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
		const auto slot = registry.get<Equipment>(item).slot;
		auto& slots = registry.get<EquipmentSlots>(entity).slots;
		using Slot = Equipment::Slot;
		if (slot == Slot::OneHanded)
		{
			if (!slots[Slot::LeftHand].has_value())
			{	// Free left hand
				slots[Slot::LeftHand] = item;
			}
			else if (!slots[Slot::RightHand].has_value())
			{	// Free right hand
				slots[Slot::RightHand] = item;
			}
			else
			{	// Neither is free
				if (slots[Slot::LeftHand].value() == slots[Slot::RightHand].value())
				{	// Same weapon wielded with both hands
					slots[Slot::RightHand] = std::nullopt;
				}
				slots[Slot::LeftHand] = item;
			}
		}
	}

	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		for (auto& [slot, equipped_item] : registry.get<EquipmentSlots>(entity).slots)
		{
			if (equipped_item == item)
				equipped_item = std::nullopt;
		}
	}

	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		for (const auto& [slot, equipped_item] : registry.get<EquipmentSlots>(entity).slots)
		{
			if (equipped_item == item)
				return true;
		}
		return false;
	}

	bool is_dual_wielding(const entt::registry& registry, const entt::entity entity)
	{
		auto slots = registry.get<EquipmentSlots>(entity).slots;
		if (!slots[Equipment::Slot::LeftHand].has_value() || !slots[Equipment::Slot::RightHand].has_value())
			return false;
		return slots[Equipment::Slot::LeftHand].value() != slots[Equipment::Slot::RightHand].value();
	}
};
