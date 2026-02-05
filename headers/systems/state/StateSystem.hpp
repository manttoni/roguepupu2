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
	int get_attributes_sum(const entt::registry& registry, const entt::entity entity, const std::vector<Attribute> attributes);

	template<typename T>
		int get_attribute(const entt::registry& registry, const entt::entity entity)
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
			assert(registry.all_of<T>(entity));
			int attribute = registry.get<T>(entity).value;
			if (registry.all_of<BuffContainer<T>>(entity))
			{
				for (const Buff<T>& buff : registry.get<BuffContainer<T>>(entity).buffs)
					attribute += buff.value;
			}
			return attribute;
		}
};
