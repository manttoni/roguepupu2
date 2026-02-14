#include "systems/combat/DamageSystem.hpp"

#include <optional>

#include "systems/rendering/VisualEffectSystem.hpp"
#include "external/entt/entt.hpp"
#include "components/Components.hpp"
#include "utils/ECS.hpp"
#include "domain/Actor.hpp"
#include "domain/Damage.hpp"
#include "domain/Effect.hpp"
#include "domain/Event.hpp"
#include "domain/Target.hpp"

namespace DamageSystem
{
	void perish(entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Dead>(entity))
			registry.emplace<Dead>(entity);
		if (registry.all_of<Solid>(entity))
			registry.erase<Solid>(entity);
		if (entity == ECS::get_player(registry))
			registry.ctx().get<GameState>().game_running = false;

		Event event = {.type = Event::Type::Death};
		event.actor.entity = entity;
		ECS::queue_event(registry, event);
	}

	void take_damage(entt::registry& registry, const entt::entity entity, const Damage::Roll& damage_roll)
	{
		if (!registry.all_of<Health>(entity))
			return;
		auto& hp = registry.get<Health>(entity).current;
		hp -= damage_roll.result;

		// This needs its own system, some AnimationSystem with queued animations
		if (registry.ctx().get<GameState>().test_run == false)
		{
			VisualEffectSystem::damage_flash(registry, entity);
		}
		if (registry.all_of<Position>(entity))
		{
			ECS::spawn_liquid(
					registry,
					registry.get<Position>(entity),
					LiquidMixture(Liquid::Type::Blood, damage_roll.result)
					);
		}

		Event event = {.type = Event::Type::TakeDamage};
		event.actor.entity = entity;
		event.damage_roll = damage_roll;
		ECS::queue_event(registry, event);

		if (hp <= 0)
			perish(registry, entity);
	}
};
