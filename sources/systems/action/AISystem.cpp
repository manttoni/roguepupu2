#include <cassert>
#include "external/entt/entt.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/action/AISystem.hpp"
#include "components/Components.hpp"
#include "systems/action/AbilitySystem.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "utils/Log.hpp"
#include "utils/ECS.hpp"

namespace AISystem
{
	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI, Position>(npc))
			return {.type = Intent::Type::DoNothing};

		return {.type = Intent::Type::DoNothing};
	}
};
