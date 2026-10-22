#pragma once
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include "game/world/Position.hpp"
#include "external/entt/fwd.hpp"

/* This namespace describes components that are created during runtime.
 * */
namespace Game::Component::Runtime
{
	struct Vision
	{
		std::vector<World::GlobalPosition> visible_positions;
		std::unordered_set<World::GlobalPosition> seen_positions;
		std::vector<entt::entity> visible_entities;
	};

	struct Inventory
	{
		std::vector<entt::entity> items;
	};
	struct EquippedItems
	{
	};
}
