#include <optional>
#include <stdexcept>
#include <cassert>
#include "systems/EquipmentSystem.hpp"
#include "Components.hpp"
#include "Utils.hpp"

namespace EquipmentSystem
{
	Slot parse_slot(const std::string& str)
	{
		if (str == "one_handed")
			return Slot::one_handed;
		else if (str == "two_handed")
			return Slot::two_handed;
		else if (str == "body")
			return Slot::body;
		else if (str == "gloves")
			return Slot::gloves;
		else if (str == "helmet")
			return Slot::helmet;
		else if (str == "boots")
			return Slot::boots;
		else if (str == "ring")
			return Slot::ring;
		else if (str == "amulet")
			return Slot::amulet;
		else if (str == "cloak")
			return Slot::cloak;

		Log::error("Unknown equipment slot: " + str);
	}

	void equip_or_unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (is_equipped(registry, entity, item))
			unequip(registry, entity, item);
		else
			equip(registry, entity, item);
	}

	void equip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!registry.all_of<Equipment>(entity) || !registry.all_of<Equippable>(item))
			return;
		if (is_equipped(registry, entity, item))
			return;

		auto& equipment = registry.get<Equipment>(entity).slots;
		auto& slot = registry.get<Equippable>(item).slot;
		if (slot == Slot::one_handed)
		{
			if (equipment[Slot::right_hand].has_value())
			{
				if (equipment[Slot::left_hand].has_value())
					equipment[Slot::right_hand] = item;
				else
					equipment[Slot::left_hand] = item;
			}
			else
				equipment[Slot::right_hand] = item;
		}
		else if (slot == Slot::two_handed)
		{
			equipment[Slot::right_hand] = item;
			equipment[Slot::left_hand] = item;
		}
		else
			equipment[slot] = item;

		assert(is_equipped(registry, entity, item));
	}

	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!is_equipped(registry, entity, item))
			return;

		auto& equipment = registry.get<Equipment>(entity);
		for (auto& [slot, equipped_item] : equipment.slots)
		{
			if (equipped_item.has_value() && equipped_item.value() == item)
				equipped_item = std::nullopt;
		}
		assert(!is_equipped(registry, entity, item));
	}

	bool is_equipped(const entt::registry& registry, const entt::entity& entity, const entt::entity& item)
	{
		if (!registry.all_of<Equipment>(entity) || !registry.all_of<Equippable>(item))
			return false;

		const auto& equipment = registry.get<Equipment>(entity);

		for (const auto& [slot, equipped_item] : equipment.slots)
		{
			if (equipped_item.has_value() && equipped_item.value() == item)
				return true;
		}
		return false;
	}
};
