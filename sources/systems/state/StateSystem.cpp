#include <stddef.h>
#include <cmath>
#include <vector>

#include "systems/state/StateSystem.hpp"
#include "components/Components.hpp"
#include "external/entt/entity/fwd.hpp"

namespace StateSystem
{
	size_t level_to_xp(const size_t level)
	{
		// temporary
		return level * 1000;
	}

	size_t xp_to_level(const size_t xp)
	{
		// temporary
		return xp / 1000;
	}

	int get_vision_range(const entt::registry& registry, const entt::entity entity)
	{
		const int perception = get_attribute<Perception>(registry, entity);
		return 30.0 * (1.0 - std::exp(-0.05 * perception));
	}
	int get_max_health(const entt::registry& registry, const entt::entity entity)
	{
		return get_attribute<Vitality>(registry, entity);
	}
	int get_max_stamina(const entt::registry& registry, const entt::entity entity)
	{
		return get_attribute<Endurance>(registry, entity);
	}
	int get_max_mana(const entt::registry& registry, const entt::entity entity)
	{
		return get_attribute<Willpower>(registry, entity);
	}

	int get_attributes_sum(const entt::registry& registry, const entt::entity entity, const std::vector<Attribute> attributes)
	{
		int sum = 0;
		for (const auto attribute : attributes)
		{
			switch (attribute)
			{
				case Attribute::Perception:
					sum += get_attribute<Perception>(registry, entity);
					break;
				case Attribute::Charisma:
					sum += get_attribute<Charisma>(registry, entity);
					break;
				case Attribute::Endurance:
					sum += get_attribute<Endurance>(registry, entity);
					break;
				case Attribute::Willpower:
					sum += get_attribute<Willpower>(registry, entity);
					break;
				case Attribute::Vitality:
					sum += get_attribute<Vitality>(registry, entity);
					break;
				case Attribute::Strength:
					sum += get_attribute<Strength>(registry, entity);
					break;
				case Attribute::Dexterity:
					sum += get_attribute<Dexterity>(registry, entity);
					break;
				case Attribute::Agility:
					sum += get_attribute<Agility>(registry, entity);
					break;
				default:
					Error::fatal("Unhandled attribute in get_attributes_sum" +
							std::to_string(static_cast<size_t>(attribute)));
			}
		}
		return sum;
	}
};
