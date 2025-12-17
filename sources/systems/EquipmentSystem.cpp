#include <string>                       // for operator==, basic_string, ope...
#include "ECS.hpp"
#include "Components.hpp"               // for Equipment, Weapon, Armor (ptr...
#include "Utils.hpp"                    // for error
#include "entt.hpp"                     // for vector, entity, null_t, find
#include "systems/EquipmentSystem.hpp"  // for Slot, equip, equip_armor, equ...

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
		if (is_equipped(registry, entity, item))
			return;

		const auto& subcategory = registry.get<Subcategory>(item).subcategory;
		if (subcategory == "weapons")
			equip_weapon(registry, entity, item);
		else if (subcategory == "armor")
			equip_armor(registry, entity, item);
	}

	void equip_weapon(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!registry.all_of<Equipment>(entity))
			return;
		auto& equipment = registry.get<Equipment>(entity);
		if (registry.all_of<TwoHanded>(item))
		{
			equipment.right_hand = item;
			equipment.left_hand = item;
			return;
		}
		if (equipment.right_hand == entt::null)
			equipment.right_hand = item;
		else
			equipment.left_hand = item;
	}

	void equip_armor(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!registry.all_of<Equipment>(entity))
			return;
		auto& equipment = registry.get<Equipment>(entity);
		equipment.armor = item;
	}

	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!is_equipped(registry, entity, item))
			return;

		auto& equipment = registry.get<Equipment>(entity);
		if (equipment.right_hand == item)
			equipment.right_hand = entt::null;
		if (equipment.left_hand == item)
			equipment.left_hand = entt::null;
		if (equipment.armor == item)
			equipment.armor = entt::null;
	}

	bool is_equipped(const entt::registry& registry, const entt::entity& entity, const entt::entity& item)
	{
		if (!registry.all_of<Equipment>(entity) || !ECS::is_equippable(registry, item))
			return false;

		const auto& equipment = registry.get<Equipment>(entity);
		if (equipment.right_hand == item || equipment.left_hand == item)
			return true;
		if (equipment.armor == item)
			return true;

		return false;
	}

	bool is_dual_wielding(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Equipment>(entity))
			return false;
		const auto equipment = registry.get<Equipment>(entity);
		if (equipment.right_hand == entt::null || equipment.left_hand == entt::null)
			return false;
		if (equipment.right_hand == equipment.left_hand)
			return false;
		return true;
	}
};
