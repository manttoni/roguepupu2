#include "systems/combat/DamageSystem.hpp"

#include <optional>

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

	}

	void take_damage(entt::registry& registry, const entt::entity entity, const Damage& damage)
	{
		if (!registry.all_of<Health>(entity))
			return;
		auto& hp = registry.get<Health>(entity).current;
		hp -= damage.amount;
		ECS::queue_event(registry, Event(
					Actor{.entity = entity},
					Effect{.type = Effect::Type::TakeDamage, .damage = damage},
					Target{}
					));
		if (hp <= 0)
			perish(registry, entity);
	}
};
