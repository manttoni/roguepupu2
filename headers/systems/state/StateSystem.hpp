#pragma once

/* StateSystem handles everything related to entities' changing state
 * Health, stamina, mana, level, attributes...
 * Level is derived value from Experience component
 * */

#include "external/entt/fwd.hpp"

namespace StateSystem
{
	size_t level_to_xp(const size_t level);
	size_t xp_to_level(const size_t xp);
	double get_vision_range(const entt::registry& registry, const entt::entity entity);
};
