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
		ECS::queue_event(registry, Event(
					Actor{.entity = entity},
					Effect{.type = Effect::Type::TakeDamage, .damage = damage},
					Target{}
					));
	}
};
