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
	std::vector<size_t> find_path(Cell& start, Cell& end)
	{
		if (start.get_cave() != end.get_cave())
			return {};
		return find_path(start.get_cave(), start.get_idx(), end.get_idx());
	}

	std::vector<size_t> find_path(Cave* cave, const size_t start, const size_t end, const bool allow_blocked_end)
	{
		if (cave == nullptr || start >= cave->get_size() || end >= cave->get_size())
			Log::error("Cave::find_path: invalid arguments");
		if (allow_blocked_end == false && cave->get_cell(end).blocks_movement())
			return {};
		std::vector<size_t> open_set = { start };
		std::map<size_t, size_t> came_from;
		std::map<size_t, double> g_score;
		std::map<size_t, double> f_score;

		g_score[start] = 0;
		f_score[start] = cave->distance(start, end);

		while (!open_set.empty())
		{
			size_t current_idx = open_set[0];
			for (const size_t cell_idx : open_set)
			{	// all open_set elements have f_score mapped
				if (f_score[cell_idx] < f_score[current_idx])
					current_idx = cell_idx;
			}

			if (current_idx == end ||
				(allow_blocked_end == true && cave->distance(current_idx, end) < MELEE_RANGE))
			{	// found optimal path from start to end
				std::vector<size_t> path;
				path.push_back(current_idx);
				while (current_idx != start)
				{	// assign the cell from where we got to to current
					current_idx = came_from[current_idx];
					path.push_back(current_idx);
				}
				std::reverse(path.begin(), path.end());
				return path;
			}

			Utils::remove_element(open_set, current_idx);
			for (const size_t neighbor_idx : cave->get_nearby_ids(current_idx, 1.5))
			{
				if (!can_move(*cave, current_idx, neighbor_idx))
					continue;
				double tentative_g_score = g_score[current_idx] + cave->distance(current_idx, neighbor_idx);
				if (g_score.count(neighbor_idx) == 0)
					g_score[neighbor_idx] = std::numeric_limits<double>::infinity();
				if (tentative_g_score < g_score[neighbor_idx])
				{
					came_from[neighbor_idx] = current_idx;
					g_score[neighbor_idx] = tentative_g_score;
					f_score[neighbor_idx] = tentative_g_score + cave->distance(neighbor_idx, end);
					if (!Utils::contains(open_set, neighbor_idx))
						open_set.push_back(neighbor_idx);
				}
			}
		}
		return {};
	}

	Cell* get_first_step(const entt::registry& registry, const entt::entity entity, Cell& dst)
	{
		Cell& src = *ECS::get_cell(registry, entity);
		const auto path = find_path(src, dst);
		if (path.empty())
			return nullptr;
		return &src.get_cave()->get_cell(path[1]);
	}

	void move(entt::registry& registry, const Actor& actor, const Target& target)
	{
		registry.emplace_or_replace<Position>(actor.entity, target.position);
		ECS::add_event(registry, {
				.type = Event::Type::Move,
				.actor = actor,
				.target = target
				});
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
