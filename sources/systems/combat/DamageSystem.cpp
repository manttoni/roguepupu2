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
	void die(entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Dead>(entity))
			registry.emplace<Dead>(entity);
		if (registry.all_of<Collision>(entity))
			registry.replace<Collision>(entity); // replace with default (false...false)
		if (entity == ECS::get_player(registry))
			registry.ctx().get<GameState>().game_running = false;

		Event event(Event::Type::Death);
		event.actor.entity = entity;
		ECS::queue_event(registry, event);
	}

	void take_damage(entt::registry& registry, const entt::entity entity, const size_t amount, const DamageType dt)
	{
		if (!registry.all_of<HitPoints>(entity))
		{
			Log::warning() << "Entity \"" << ECS::get_name(registry, entity) << "\" has no HitPoints component";
			return;
		}

		auto& hp = registry.get<HitPoints>(entity).value;
		hp -= amount;

		VisualEffectSystem::damage_flash(registry, entity);

		Event event(Event::Type::TakeDamage);
		event.actor.entity = entity;
		event.damage_roll = {.amount = amount, .damage_type = dt};
		ECS::queue_event(registry, event);

		if (hp <= 0)
			die(registry, entity);
	}
};
