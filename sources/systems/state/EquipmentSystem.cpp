#include "systems/state/EquipmentSystem.hpp"
#include "domain/Enum.hpp"

namespace EquipmentSystem
{
	using namespace Component;
	using namespace Tag;
	using namespace List;
	using Enum::WeaponProperty;

	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		assert(registry.all_of<EquippedItems>(entity));
		assert(registry.all_of<Equipment>(item));
		return registry.get<EquippedItems>(entity).contains(item);
	}

	bool valid_dual_wield(const entt::registry& registry, const std::vector<entt::entity>& weapons)
	{
		if (weapons.size() != 2)
			return false;

		for (const auto w : weapons)
		{
			if (ECS::weapon_has_property(registry, w, WeaponProperty::TwoHanded)
					|| !ECS::weapon_has_property(registry, w, WeaponProperty::Light))
				return false;
		}
		return true;
	}

	bool valid_hand_loadout(const entt::registry& registry, const EquippedItems& equipped_items)
	{
		const auto weapons = ECS::get<Weapon>(registry, equipped_items);
		const auto shields = ECS::get<Shield>(registry, equipped_items);
		const auto w_size = weapons.size();
		const auto s_size = shields.size();
		const auto total_size = w_size + s_size;

		switch (total_size)
		{
			case 0:
			case 1:
				return true;
			case 2:
				switch (s_size)
				{
					case 0:
						return valid_dual_wield(registry, weapons);
					case 1:
						return !ECS::weapon_has_property(registry, weapons.front(), WeaponProperty::TwoHanded);
					default:
						return false;
				}
			default:
				return false;
		}
	}

	bool valid_loadout(const entt::registry& registry, const EquippedItems& equipped_items)
	{
		// TODO: Validate each item, they must have one of these tags
		if (!valid_hand_loadout(registry, equipped_items)
				|| ECS::count<Helmet>(registry, equipped_items) > 1
				|| ECS::count<Armor>(registry, equipped_items) > 1
				|| ECS::count<Cloak>(registry, equipped_items) > 1
				|| ECS::count<Amulet>(registry, equipped_items) > 1
				|| ECS::count<Shield>(registry, equipped_items) > 1
				|| ECS::count<Gloves>(registry, equipped_items) > 1
				|| ECS::count<Belt>(registry, equipped_items) > 1
				|| ECS::count<Boots>(registry, equipped_items) > 1
				|| ECS::count<Quiver>(registry, equipped_items) > 1
				|| ECS::count<Ring>(registry, equipped_items) > 2)
		{
			return false;
		}
		return true;
	}

	bool can_equip(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		auto copy = registry.get<EquippedItems>(entity);
		copy.push_back(item);
		return valid_loadout(registry, copy);
	}

	bool can_unequip(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!is_equipped(registry, entity, item))
			return false;

		return true; // Probably always true. Cursed items?
	}

	void equip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!can_equip(registry, entity, item))
			return;

		registry.get<EquippedItems>(entity).push_back(item);
		ECS::queue_event(
				registry,
				EquipEvent{
				.entity = entity,
				.equipment = item
				});
	}

	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!can_unequip(registry, entity, item))
			return;

		auto& equipped_items = registry.get<EquippedItems>(entity);
		equipped_items.remove(item);
	}

	/* TODO: "force equipping"
	void replace_equipment(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{}
	*/

	std::vector<entt::entity> get_weapons(const entt::registry& registry, const entt::entity entity)
	{
		const auto& equipped = registry.get<EquippedItems>(entity).values;
		return ECS::get<Weapon>(registry, equipped);
	}
}

