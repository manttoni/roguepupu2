#include "systems/combat/DamageSystem.hpp"

#include "systems/rendering/VisualEffectSystem.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "domain/Event.hpp"


using namespace Component::Tag;
using namespace Component::Value;

namespace DamageSystem
{
	void die(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace_or_replace<Dead>(entity);
		registry.emplace_or_replace<CollisionMovement>(entity, false);
		registry.emplace_or_replace<CollisionVision>(entity, false);
		if (entity == ECS::get_player(registry))
			registry.ctx().get<GameState>().game_running = false;

		ECS::queue_event(
				registry,
				DeathEvent{
				.entity = entity
				});
	}

	void take_damage(entt::registry& registry, const entt::entity entity, const size_t amount, const Enum::DamageType damage_type)
	{
		if (!registry.all_of<Component::Resource::HitPoints>(entity))
		{
			Log::warning() << "Entity \"" << ECS::get_name(registry, entity) << "\" has no HitPoints component";
			return;
		}

		auto& hp = registry.get<Component::Resource::HitPoints>(entity).current;
		hp -= amount;

		VisualEffectSystem::damage_flash(registry, entity);

		ECS::queue_event(
				registry,
				TakeDamageEvent{
				.target = entity,
				.damage_type = damage_type,
				.amount = amount
				});

		if (hp <= 0)
			die(registry, entity);
	}
};
