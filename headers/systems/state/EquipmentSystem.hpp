#pragma once

#include "external/entt/entt.hpp"
#include "components/Component.hpp"
#include "utils/ECS.hpp"

namespace EquipmentSystem
{
	using namespace Component;
	using namespace Tag;
	using namespace List;

	std::vector<entt::entity> get_weapons(const entt::registry& registry, const entt::entity entity);
	bool is_equipped(const entt::registry& registry, const entt::entity entity, const entt::entity item);

	/*template<typename T>
		bool can_equip(const entt::registry& registry, const entt::entity entity, const entt::entity item)
		{
			if (is_equipped(registry, entity, item) || !registry.all_of<T>(item) || !registry.all_of<EquippedItems>(entity))
				return false;
			const auto& equipped_items = registry.get<EquippedItems>(entity);
			const auto count = ECS::count<T>(registry, equipped_items);
			if (std::same_as<T, Weapon>)
			{
				if (count == 0)
					return true;
				if (count == 2)
					return false;
				assert(count == 1);
				// 1 weapon equipped
				if (ECS::weapon_has_property(registry, item, Enum::WeaponProperty::TwoHanded))
					return false;
				const auto weapons = get_weapons(registry, entity);
				assert(weapons.size() == 1);
				const auto equipped_weapon = weapons.front();
				// Both weapons need Light property for dual-wielding
				return ECS::weapon_has_property(registry, item, Enum::WeaponProperty::Light)
					&& ECS::weapon_has_property(registry, equipped_weapon, Enum::WeaponProperty::Light);
			}

			if (std::same_as<T, Ring>)
				return count < 2;
			return count < 1;
		}*/

	bool can_equip(const entt::registry& registry, const entt::entity entity, const entt::entity item);
	bool can_unequip(const entt::registry& registry, const entt::entity entity, const entt::entity item);
	void equip(entt::registry& registry, const entt::entity entity, const entt::entity equipment);
	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity equipment);
}
