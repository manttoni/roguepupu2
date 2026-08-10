#include <stddef.h>
#include <cmath>
#include <vector>

#include "utils/Log.hpp"
#include "systems/state/StateSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "components/Component.hpp"
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


	int get_attribute_modifier(const int attribute)
	{
		return (attribute - 10) / 2;
	}

	/* Similar to get_stat<AC>, but give +dexmod and if unarmored give +10
	 * */
	int get_armor_class(const entt::registry& registry, const entt::entity entity)
	{
		(void) registry; (void) entity;
		return 10;
	}
};
