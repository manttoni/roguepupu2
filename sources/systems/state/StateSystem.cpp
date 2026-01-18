#include "external/entt/entt.hpp"
#include "systems/state/StateSystem.hpp"

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
};
