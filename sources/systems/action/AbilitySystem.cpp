#include <cassert>
#include <string>
#include "external/entt.hpp"
#include "domain/Ability.hpp"
#include "domain/Target.hpp"
#include "domain/Effect.hpp"
#include "infrastructure/GameState.hpp"

namespace AbilitySystem
{
	bool on_cooldown(const entt::registry& registry, const Ability& ability)
	{
		if (ability.last_used == 0)
			return false;

		const size_t turn_number = registry.ctx().get<GameState>().turn_number;
		return ability.last_used + ability.cooldown >= turn_number;
	}

	bool has_ability(const entt::registry& registry, const entt::entity entity, const std::string& ability_id)
	{
		if (!registry.all_of<Abilities>(entity))
			return false;
		return registry.get<Abilities>(entity).abilities.contains(ability_id);
	}

	void use_ability(entt::registry& registry, const entt::entity user, const std::string& ability_id, const Target& target)
	{
		assert(target.type != Target::Type::None);
		assert(user != entt::null);
		assert(has_ability(registry, user, ability_id);
		const size_t turn_number = registry.ctx().get<GameState>().turn_number;
		Ability& ability = registry.get<Abilities>(user).abilities[ability_id];
		const Effect& effect = ability.effect;
		EffectSystem::resolve_effect(registry, effect, target);
		ability.last_used = turn_number;
	}
};
