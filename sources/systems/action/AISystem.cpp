#include "domain/Action.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "systems/action/AISystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "systems/combat/CombatSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Random.hpp"

using namespace Component::Tag;
using namespace Component::List;
using namespace Component::Value;

namespace AISystem
{
	std::vector<Action> get_actions(const entt::registry& registry, const entt::entity npc)
	{
		assert((registry.all_of<Position, AIBehaviors, Actor>(npc)));
		const auto& ai_behaviors = registry.get<AIBehaviors>(npc);
		Log::debug() << "Getting AI Action of: " << registry.get<Name>(npc);

		for (const auto behavior : ai_behaviors)
		{
			// behaviors are entities in a vector
			// they have Tags related to AI
			// for example <AIBehaviorAggressive, AIBehaviorMushroomPicker>
			if (registry.all_of<Component::Tag::AIBehavior>(behavior)) // every 'behavior' entity should have this tag
				Log::debug() << "I have a behavior: " << registry.get<Name>(behavior);
		}
		return {};
	}
};
