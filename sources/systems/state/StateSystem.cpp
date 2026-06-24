#include <stddef.h>
#include <cmath>
#include <vector>

#include "systems/state/StateSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
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

	int get_initiative(const entt::registry& registry, const entt::entity entity)
	{
		return Dice(1, 20).roll() + get_attribute_modifier<Dexterity>(registry, entity) + get_stat<Initiative>(registry, entity);
	}
	int get_attribute_modifier(const int attribute)
	{
		return (attribute - 10) / 2;
	}

	/* Similar to get_stat<AC>, but give +dexmod and if unarmored give +10
	 * */
	int get_armor_class(const entt::registry& registry, const entt::entity entity)
	{
		const auto dex_modifier = get_attribute_modifier<Dexterity>(registry, entity);
		const auto body_armor = EquipmentSystem::get_equipment_at(registry, entity, EquipmentSlot::Body);

		int AC = get_stat<ArmorClass>(registry, entity); // get AC from equipment and buffs

		if (body_armor == entt::null)
			AC += 10 + dex_modifier;
		else if (registry.all_of<MaxDexMod>(body_armor))
			AC += std::min(dex_modifier, registry.get<MaxDexMod>(body_armor).value);
		else
			AC += dex_modifier;

		return AC;
	}
};
