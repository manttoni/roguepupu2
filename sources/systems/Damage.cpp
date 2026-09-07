#include "systems/Damage.hpp"


#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "domain/Event.hpp"




namespace System::Damage

{
	void die(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace_or_replace<Component::Tag::Dead>(entity);
		registry.emplace_or_replace<Component::Value::CollisionMovement>(entity, false);
		registry.emplace_or_replace<Component::Value::CollisionVision>(entity, false);
		if (entity == ECS::get_player(registry))
			registry.ctx().get<GameState>().game_running = false;

		ECS::queue_event(
				registry,
				Domain::Event::Death{
				.entity = entity
				});
	}

	void take_damage(entt::registry& registry, const entt::entity entity, const size_t amount, const Domain::Enum::DamageType damage_type)
	{
		if (!registry.all_of<Component::Resource::HitPoints>(entity))
		{
			Log::warning() << "Entity \"" << ECS::get_name(registry, entity) << "\" has no HitPoints component";
			return;
		}

		auto& hp = registry.get<Component::Resource::HitPoints>(entity).current;
		hp -= amount;

		ECS::queue_event(
				registry,
				Domain::Event::TakeDamage{
				.target = entity,
				.damage_type = damage_type,
				.amount = amount
				});

		if (hp <= 0)
			die(registry, entity);
	}
};
