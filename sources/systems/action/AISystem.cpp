#include "external/entt/entt.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "systems/action/AISystem.hpp"
#include "domain/Intent.hpp"
#include "external/entt/entity/fwd.hpp"
#include "components/Components.hpp"
#include "utils/ECS.hpp"
#include "utils/Random.hpp"

struct Position;

namespace AISystem
{
	bool idle_wander(const entt::registry& registry, Intent& intent)
	{
		if (!intent.actor.position.is_valid())
			return false;

		const auto& cave = ECS::get_cave(registry, intent.actor.position);
		const auto nearby_positions = cave.get_nearby_positions(intent.actor.position, 1.5);
		if (nearby_positions.empty())
			return false;

		intent.target.position = Random::get_random_element(nearby_positions);
		intent.type = Intent::Type::Move;
		return true;
	}

	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI>(npc))
			return {.type = Intent::Type::DoNothing};

		const auto& ai = registry.get<AI>(npc);
		Intent intent;
		intent.actor.entity = npc;
		if (registry.all_of<Position>(npc))
			intent.actor.position = registry.get<Position>(npc);

		if (ai.idle_wander == true && idle_wander(registry, intent) == true)
			return intent;

		intent.type = Intent::Type::DoNothing;
		return intent;
	}
};
