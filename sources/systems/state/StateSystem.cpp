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

	double get_vision_range(const entt::registry& registry, const entt::entity entity)
	{	(void) registry; (void) entity;
		// temporary
		return 13;
	}

	int get_max_health(const entt::registry& registry, const entt::entity entity)
	{
		assert(registry.all_of<Vitality>(entity));
		const auto& vitality = registry.get<Vitality>(entity);
		return vitality.value;
	}
	int get_max_stamina(const entt::registry& registry, const entt::entity entity)
	{
		assert(registry.all_of<Endurance>(entity));
		const auto& endurance = registry.get<Endurance>(entity);
		return endurance.value;
	}
	int get_max_mana(const entt::registry& registry, const entt::entity entity)
	{
		assert(registry.all_of<Willpower>(entity));
		const auto& willpower = registry.get<Willpower>(entity);
		return willpower.value;
	}

};
