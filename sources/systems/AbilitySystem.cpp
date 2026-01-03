#include <cassert>
#include "systems/AbilitySystem.hpp"
#include "systems/EffectSystem.hpp"
#include "entt.hpp"
#include "GameState.hpp"

namespace AbilitySystem
{
	bool on_cooldown(const entt::registry& registry, const Ability& ability)
	{
		if (ability.last_used == 0)
			return false;

		const size_t turn_number = registry.ctx().get<GameState>().turn_number;
		return ability.last_used + ability.cooldown >= turn_number;
	}

	void use_ability(entt::registry& registry, Ability& ability, const Target& target)
	{
		assert(target.type != Target::Type::None);
		const size_t turn_number = registry.ctx().get<GameState>().turn_number;
		const Effect& effect = ability.effect;
		EffectSystem::resolve_effect(registry, effect, target);
		ability.last_used = turn_number;
	}
};
