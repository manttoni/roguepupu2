#include <vector>
#include "utils/ECS.hpp"
#include "external/entt/entt.hpp"
#include "systems/position/PositionSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "domain/Cell.hpp"
#include "domain/Cave.hpp"

#define MELEE_RANGE 1.5

namespace MovementSystem
{
	std::vector<size_t> find_path(const Cave& cave, const size_t start, const size_t end, const bool allow_blocked_end)
	{
		if (start >= cave.get_size() || end >= cave.get_size())
			Log::error("Find path idx out of bounds");
		if (allow_blocked_end == false && cave.get_cell(end).get_type() == Cell::Type::Rock)
			return {};
		std::vector<size_t> open_set = { start };
		std::map<size_t, size_t> came_from;
		std::map<size_t, double> g_score;
		std::map<size_t, double> f_score;

		g_score[start] = 0;
		f_score[start] = cave.distance(start, end);

		while (!open_set.empty())
		{
			size_t current_idx = open_set[0];
			for (const size_t cell_idx : open_set)
			{	// all open_set elements have f_score mapped
				if (f_score[cell_idx] < f_score[current_idx])
					current_idx = cell_idx;
			}

			if (current_idx == end ||
				(allow_blocked_end == true && cave.distance(current_idx, end) < MELEE_RANGE))
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

			open_set.erase(std::find(open_set.begin(), open_set.end(), current_idx)); // is found
			for (const size_t neighbor_idx : cave.get_nearby_ids(current_idx, 1.5))
			{
				if (!can_move(cave, current_idx, neighbor_idx))
					continue;
				double tentative_g_score = g_score[current_idx] + cave.distance(current_idx, neighbor_idx);
				if (g_score.count(neighbor_idx) == 0)
					g_score[neighbor_idx] = std::numeric_limits<double>::infinity();
				if (tentative_g_score < g_score[neighbor_idx])
				{
					came_from[neighbor_idx] = current_idx;
					g_score[neighbor_idx] = tentative_g_score;
					f_score[neighbor_idx] = tentative_g_score + cave.distance(neighbor_idx, end);
					auto it = std::find(open_set.begin(), open_set.end(), neighbor_idx);
					if (it == open_set.end())
						open_set.push_back(neighbor_idx);
				}
			}
		}
		return {};
	}

	std::vector<size_t> find_path(const entt::registry& registry, const Position& start, const Position& end, const bool allow_blocked_end)
	{
		if (start.cave_idx != end.cave_idx)
			return {};
		const auto& cave = PositionSystem::get_cave(registry, start);
		return find_path(cave, start.cell_idx, end.cell_idx, allow_blocked_end);
	}

	Position get_first_step(const entt::registry& registry, const Position& start, const Position& end)
	{
		const auto path = find_path(registry, start, end);
		assert(!path.empty() && "asking for non-existent first step");
		return Position(path[1], start.cave_idx);
	}

	void move(entt::registry& registry, const entt::entity entity, const Position& position)
	{
		registry.emplace_or_replace<Position>(entity, position);
		Event move_event;
		move_event.actor.entity = entity;
		move_event.effect.type = Effect::Type::Move;
		move_event.target.position = position;
		ECS::queue_event(registry, move_event);
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
