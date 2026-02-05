#include "external/entt/entt.hpp"
#include "systems/action/AISystem.hpp"
#include "domain/Intent.hpp"
#include "external/entt/entity/fwd.hpp"
#include "components/Components.hpp"
#include "utils/ECS.hpp"

struct Position;

namespace AISystem
{
	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI, Position>(npc))
			return {.type = Intent::Type::DoNothing};

		const auto& ai = registry.get<AI>(npc);
		const auto& position = registry.get<Position>(npc);
		const auto& cave = ECS::get_cave(registry, position);
		Intent intent;
		intent.actor = {.entity = npc, .position = position};

		if (ai.idle_wander == true)
		{
			const auto nearby_positions = cave.get_nearby_positions(position);
			if (!nearby_positions.empty())
			{
				intent.target.position = Random::get_random_element(nearby_positions);
				intent.type = Intent::Type::Move;
				return intent;
			}
		}

		return {.type = Intent::Type::DoNothing};
	}
};
