#pragma once

#include "databases/EntityDatabase.hpp"
#include "game/event/Event.hpp"
#include "external/entt/fwd.hpp"
#include "game/component/List.hpp"
#include "game/entity/Definition.hpp"

namespace Game::System::Loot
{
	struct Entry
	{
		Entity::Definition::ID id;
		double priority;
	};
	Event::List generate(
		entt::registry& registry,
		const EntityDatabase& database,
		entt::entity recipient,
		const Component::List::Base<Entity::Definition::ID>* loot_tables);
}
