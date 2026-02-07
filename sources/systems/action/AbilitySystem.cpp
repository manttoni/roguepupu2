#include <stddef.h>
#include <cassert>
#include <string>
#include <map>

#include "systems/action/EffectSystem.hpp"
#include "external/entt/entt.hpp"
#include "domain/Ability.hpp"
#include "domain/Actor.hpp"
#include "domain/Event.hpp"
#include "utils/ECS.hpp"
#include "components/Components.hpp"
#include "domain/Position.hpp"
#include "domain/Target.hpp"
#include "external/entt/entity/fwd.hpp"

struct Effect;

namespace AbilitySystem
{
	bool on_cooldown(const entt::registry& registry, const Ability& ability)
	{
		if (ability.last_used == 0)
			return false;

		const size_t turn_number = ECS::get_turn_number(registry);
		return ability.last_used + ability.cooldown >= turn_number;
	}

	bool has_ability(const entt::registry& registry, const entt::entity entity, const std::string& ability_id)
	{
		if (!registry.all_of<Abilities>(entity))
			return false;
		return registry.get<Abilities>(entity).abilities.contains(ability_id);
	}

	/* Follow same formula Actor -> Effect -> Target
	 * Just convert ability_id into Effect
	 * */
	void use_ability(entt::registry& registry, const Actor& actor, const std::string& ability_id, const Target& target)
	{
		(void) registry; (void) actor; (void) ability_id; (void) target;
	}
};
