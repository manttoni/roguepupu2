#include "systems/combat/DamageSystem.hpp"
#include "systems/rendering/VisualEffectSystem.hpp"
#include "infrastructure/GameLogger.hpp"
#include "infrastructure/GameState.hpp"
#include "external/entt/entt.hpp"
#include "components/Components.hpp"
#include "utils/ECS.hpp"

namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const Damage& damage)
	{
		if (!registry.all_of<Health>(entity))
			return;
		auto& hp = registry.get<Health>(entity).current;
		hp -= damage.amount;
		Event damage_event;
		damage_event.effect.type = Effect::Type::Damage;
		damage_event.effect.damage = damage;
		damage_event.target.entity = entity;
		ECS::queue_event(registry, damage_event);
		VisualEffectSystem::damage_flash(registry, entity);
		if (hp <= 0)
		{
			Event death_event;
			death_event.effect.type = Effect::Type::Death;
			death_event.target.entity = entity;
			ECS::queue_event(registry, death_event);
		}
	}
};
