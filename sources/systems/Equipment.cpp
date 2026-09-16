#include "systems/Equipment.hpp"
#include "domain/Enum.hpp"


namespace System::Equipment
{
	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		assert(registry.all_of<Component::List::EquippedItems>(entity));
		assert(registry.all_of<Component::Tag::Equipment>(item));
		return registry.get<Component::List::EquippedItems>(entity).contains(item);
	}

	bool valid_dual_wield(const entt::registry& registry, const std::vector<entt::entity>& weapons)
	{
		if (weapons.size() != 2)
			return false;

		for (const auto w : weapons)
		{
			if (ECS::weapon_has_property(registry, w, Domain::Enum::WeaponProperty::TwoHanded)
					|| !ECS::weapon_has_property(registry, w, Domain::Enum::WeaponProperty::Light))
				return false;
		}
		return true;
	}

	bool valid_hand_loadout(const entt::registry& registry, const Component::List::EquippedItems& equipped_items)
	{
		const auto weapons = ECS::get<Component::Tag::Weapon>(registry, equipped_items);
		const auto shields = ECS::get<Component::Tag::Shield>(registry, equipped_items);
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
						return !ECS::weapon_has_property(registry, weapons.front(), Domain::Enum::WeaponProperty::TwoHanded);
					default:
						return false;
				}
			default:
				return false;
		}
	}

	bool valid_loadout(const entt::registry& registry, const Component::List::EquippedItems& equipped_items)
	{
		// TODO: Validate each item, they must have one of these tags
		if (!valid_hand_loadout(registry, equipped_items)
				|| ECS::count<Component::Tag::Helmet>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Armor>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Cloak>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Amulet>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Shield>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Gloves>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Belt>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Boots>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Quiver>(registry, equipped_items) > 1
				|| ECS::count<Component::Tag::Ring>(registry, equipped_items) > 2)
		{
			return false;
		}
		return true;
	}

	bool can_equip(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		auto copy = registry.get<Component::List::EquippedItems>(entity);
		copy.values.push_back(item);
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

		registry.get<Component::List::EquippedItems>(entity).values.push_back(item);
		ECS::queue_event(
				registry,
				Domain::Event::Equip{
				.entity = entity,
				.equipment = item
				});
	}

	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		if (!can_unequip(registry, entity, item))
			return;

		auto& equipped_items = registry.get<Component::List::EquippedItems>(entity);
		equipped_items.remove(item);
	}

	/* TODO: "force equipping"
	   void replace_equipment(entt::registry& registry, const entt::entity entity, const entt::entity item)
	   {}
	   */

	std::vector<entt::entity> get_weapons(const entt::registry& registry, const entt::entity entity)
	{
		const auto& equipped = registry.get<Component::List::EquippedItems>(entity).values;
		return ECS::get<Component::Tag::Weapon>(registry, equipped);
	}
}

