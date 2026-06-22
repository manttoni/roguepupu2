#pragma once

/* StateSystem handles everything related to entities' changing state
 * Health, stamina, mana, level, attributes...
 * Level is derived value from Experience component
 * Almost everything is basically derived from Components + temporary effects
 * */

#include "external/entt/entt.hpp"
#include "components/Components.hpp"
#include "domain/Attribute.hpp"

namespace StateSystem
{
	// Calculating level and xp
	size_t level_to_xp(const size_t level);
	size_t xp_to_level(const size_t xp);

	// Calculating stats
	/*int get_max_stamina(const entt::registry& registry, const entt::entity entity);
	int get_max_mana(const entt::registry& registry, const entt::entity entity);*/

	// Change this to darkvision based on race
	//int get_vision_range(const entt::registry& registry, const entt::entity entity);

	// Derived
	int get_initiative(const entt::registry& registry, const entt::entity entity);
	int get_armor_class(const entt::registry& registry, const entt::entity entity);
	int get_attribute_modifier(const int attribute);

	/*template<typename T>
		void assert_attribute_type()
		{
			static_assert(
					std::is_same_v<T, Perception> ||
					std::is_same_v<T, Charisma> ||
					std::is_same_v<T, Endurance> ||
					std::is_same_v<T, Willpower> ||
					std::is_same_v<T, Vitality> ||
					std::is_same_v<T, Strength> ||
					std::is_same_v<T, Dexterity> ||
					std::is_same_v<T, Agility>,
					"T must be an allowed attribute component"
					);
		}*/

	// Find from entity and all it's equipped items the component T,
	// if they are in form 'struct T { int value; ... };', then this function will sum all values
	template<typename T>
		int get_stat(const entt::registry& registry, const entt::entity entity)
		{
			if (!registry.all_of<T>(entity))
				Error::fatal("Wrong use of get_stat or get_modifier. Entity must have T component");
			int stat = registry.get<T>(entity).value;
			if (registry.all_of<EquipmentSlots>(entity))
			{
				const auto equipped_items = registry.get<EquipmentSlots>(entity).equipped_items;
				for (const auto [slot, item] : equipped_items)
					if (registry.all_of<T>(item))
						stat += registry.get<T>(item).value; // Crash if has no value member
			}
			// invent enchantment/potion systems
			return stat;
		}

	template<typename T>
		int get_attribute_modifier(const entt::registry& registry, const entt::entity entity)
		{
			const int stat = get_stat<T>(registry, entity);
			return get_attribute_modifier(stat);
		}

	/* Remove the old one, which used a different method than get_stat
	 * The idea was to make a container for all buffs, including equipment and spells/potions
	 * get_stat handles them separately
	template<typename T>
		int get_attribute(const entt::registry& registry, const entt::entity entity)
		{
			assert_attribute_type<T>();
			int attribute = registry.get<T>(entity).value;
			if (registry.all_of<BuffContainer<T>>(entity))
			{
				for (const Buff<T>& buff : registry.get<BuffContainer<T>>(entity).buffs)
					attribute += buff.value;
			}
			return attribute;
		}

	template<typename T>
		void add_buff(entt::registry& registry, const entt::entity entity, const Buff<T>& buff)
		{
			assert_attribute_type<T>();
			if (!registry.all_of<BuffContainer<T>>(entity))
				registry.emplace<BuffContainer<T>>(entity);
			registry.get<BuffContainer<T>>(entity).buffs.push_back(buff);
		}

	template<typename T>
		void remove_buff(entt::registry& registry, const entt::entity entity, const Buff<T>& buff)
		{
			assert_attribute_type<T>();
			assert(registry.all_of<BuffContainer<T>>(entity));
			auto& buffs = registry.get<BuffContainer<T>>(entity).buffs;
			auto it = std::find(buffs.begin(), buffs.end(), buff);
			assert(it != buffs.end());
			buffs.erase(it);
		}*/

};
