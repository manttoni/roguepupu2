#include <curses.h>                    // for KEY_DOWN, KEY_LEFT, KEY_RIGHT
#include "ECS.hpp"
#include "Cave.hpp"                    // for Cave
#include "Cell.hpp"                    // for Cell
#include "Components.hpp"              // for Position
#include "Utils.hpp"                   // for Vec2
#include "entt.hpp"                    // for size_t, map, entity, registry
#include "systems/MovementSystem.hpp"  // for move, movement_key_pressed
#include "systems/ActionSystem.hpp"

namespace MovementSystem
{
	void move(entt::registry& registry, entt::entity actor, Cell* target_cell)
	{
		auto& position = registry.get<Position>(actor);
		position.cell = target_cell;
		ActionSystem::use_action(registry, actor);
		// use fatigue also, not implemented yet
	}

	bool can_move(Cave& cave, const size_t from_idx, const size_t to_idx)
	{
		const auto& cells = cave.get_cells();
		if (from_idx >= cells.size() || to_idx >= cells.size())
			return false;
		const auto& to = cells[to_idx];
		if (to.blocks_movement()) // can't move to "to"
			return false;

		const auto width = cave.get_width();
		int fy = from_idx / width;
		int fx = from_idx % width;
		int ty = to_idx / width;
		int tx = to_idx % width;

		if (abs(fy - ty) > 1 || abs(fx - tx) > 1) // is not a neighbor
			return false;

		// there is access from "from" to "to" if they are on same x or y axis
		if (fy == ty || fx == tx)
			return true;

		// there is access diagonally if there is no corner to go around
		const auto& corner1 = cells[fy * width + tx];
		const auto& corner2 = cells[ty * width + fx];
		if (corner1.blocks_movement() || corner2.blocks_movement())
			return false;
		return true;
	}
};
