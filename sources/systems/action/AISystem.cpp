#include "external/entt/entt.hpp"
#include "systems/action/AISystem.hpp"
#include "domain/Intent.hpp"
#include "external/entt/entity/fwd.hpp"
#include "components/Components.hpp"

//struct AI;
struct Position;

namespace AISystem
{
	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI, Position>(npc))
			return {.type = Intent::Type::DoNothing};

		return {.type = Intent::Type::DoNothing};
	}
};
