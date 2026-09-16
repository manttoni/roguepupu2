#include "domain/Action.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "systems/AI.hpp"
#include "systems/Vision.hpp"
#include "systems/Movement.hpp"
#include "systems/Equipment.hpp"
#include "systems/Combat.hpp"
#include "utils/ECS.hpp"
#include "utils/Random.hpp"



namespace System::AI
{
	std::vector<Domain::Action::Any> get_actions(const entt::registry& registry, const entt::entity npc)
	{
		assert((registry.all_of<Domain::Position, Component::List::AIBehaviors, Component::Tag::Actor>(npc)));
		const auto& ai_behaviors = registry.get<Component::List::AIBehaviors>(npc);
		Log::debug() << "Getting AI Action of: " << registry.get<Component::Value::Name>(npc);

		for (const auto behavior : ai_behaviors.values)
		{
			// behaviors are entities in a vector
			// they have Tags related to AI
			// for example <AIBehaviorAggressive, AIBehaviorMushroomPicker>
			if (registry.all_of<Component::Tag::AIBehavior>(behavior)) // every 'behavior' entity should have this tag
				Log::debug() << "I have a behavior: " << registry.get<Component::Value::Name>(behavior);
		}
		return {};
	}
};
