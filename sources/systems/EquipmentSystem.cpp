#include <string>                       // for operator==, basic_string, ope...
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

		if (registry.all_of<Weapon>(item))
			equip_weapon(registry, entity, item);
		else if (registry.all_of<Armor>(item))
			equip_armor(registry, entity, item);
	}

	void equip_weapon(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		assert(entity != entt::null);
		auto& equipment = registry.get<Equipment>(entity);
		const auto& weapon_component = registry.get<Weapon>(item);
		const auto& properties = weapon_component.properties;
		if (std::find(properties.begin(), properties.end(), "two-handed") != properties.end())
		{
			equipment.right_hand = item;
			equipment.left_hand = item;
			return;
		}
		if (equipment.right_hand == entt::null || registry.all_of<Armor>(equipment.left_hand))
		{	// Equip in right hand if its free or left hand has a shield
			equipment.right_hand = item;
			return;
		}
		equipment.left_hand = item;
		const auto& right_properties = registry.get<Weapon>(equipment.right_hand).properties;
		const bool right_light = std::find(right_properties.begin(), right_properties.end(), "light") != right_properties.end();
		const bool item_light = std::find(properties.begin(), properties.end(), "light") != properties.end();

		// If one is not light, unequip right weapon
		if (right_light == false || item_light == false)
			unequip(registry, entity, equipment.right_hand);
	}

	void equip_armor(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
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

		if (equipment.right_hand == entt::null && !registry.all_of<Armor>(equipment.left_hand))
		{
			equipment.right_hand = equipment.left_hand;
			equipment.left_hand = entt::null;
		}
		assert(!is_equipped(registry, entity, item));
	}

	bool is_equipped(const entt::registry& registry, const entt::entity& entity, const entt::entity& item)
	{
		if (!registry.all_of<Equipment>(entity) || !registry.any_of<Weapon, Armor>(item))
			return false;

		const auto& equipment = registry.get<Equipment>(entity);
		if (equipment.right_hand == item || equipment.left_hand == item)
			return true;
		if (equipment.armor == item)
			return true;

		return false;
	}
};
