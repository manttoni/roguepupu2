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
	size_t level_to_xp(const size_t level);
	size_t xp_to_level(const size_t xp);
	int get_max_health(const entt::registry& registry, const entt::entity entity);
	int get_max_stamina(const entt::registry& registry, const entt::entity entity);
	int get_max_mana(const entt::registry& registry, const entt::entity entity);
	int get_vision_range(const entt::registry& registry, const entt::entity entity);
	int get_initiative(const entt::registry& registry, const entt::entity entity);

	template<typename T>
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
		}

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
		}
};
