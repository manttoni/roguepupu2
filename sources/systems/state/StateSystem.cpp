#include <cmath>
#include "external/entt/entt.hpp"
#include "systems/state/StateSystem.hpp"
#include "components/Components.hpp"

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
};
