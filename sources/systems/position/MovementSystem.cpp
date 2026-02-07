#include <vector>
#include <cassert>
#include "utils/ECS.hpp"
#include "external/entt/entt.hpp"
#include "systems/position/MovementSystem.hpp"
#include "domain/Cell.hpp"
#include "domain/Cave.hpp"
#include "utils/ECS.hpp"
#include "utils/Vec2.hpp"
#include "utils/Error.hpp"

namespace MovementSystem
{

	/* A cell blocks movement if it is Rock or there is a Solid entity
	 * */
	bool blocks_movement(const entt::registry& registry, const Position& position)
	{
		if (ECS::get_cell(registry, position).get_type() == Cell::Type::Rock)
			return true;
		for (const auto entity : ECS::get_entities(registry, position))
		{
			if (registry.any_of<Solid>(entity))
				return true;
		}
		return false;
	}

	/* Entities can move to neighboring cells,
	 * even diagonally if there is no corner to go around.
	 * */
	bool can_move(const entt::registry& registry, const Position& from_pos, const Position& to_pos)
	{
		const auto& cave = ECS::get_cave(registry, from_pos);
		assert(from_pos.cave_idx == to_pos.cave_idx);
		assert(from_pos.cell_idx < cave.get_area());
		assert(to_pos.cell_idx < cave.get_area());

		if (blocks_movement(registry, to_pos))
			return false;

		const auto cave_size = cave.get_size();
		const Vec2 from(from_pos.cell_idx, cave_size);
		const Vec2 to(to_pos.cell_idx, cave_size);

		if (abs(from.y - to.y) > 1 || abs(from.x - to.x) > 1) // is not a neighbor
			return false;

		// there is access from "from" to "to" if they are on same x or y axis
		if (from.y == to.y || from.x == to.x)
			return true;

		// there is access diagonally if there is no corner to go around
		const auto corner1 = Position(from.y * cave_size + to.x, cave.get_idx());
		const auto corner2 = Position(to.y * cave_size + from.x, cave.get_idx());
		if (blocks_movement(registry, corner1) || blocks_movement(registry, corner2))
			return false;
		return true;
	}

	/* A* algorithm to find path from start position to end position.
	 * If blocked end is allowed, AI can use this to conveniently move
	 * towards a cell even if it is blocked.
	 * */
	std::vector<Position> find_path(const entt::registry& registry, const Position& start, const Position& end, const bool allow_blocked_end)
	{
		assert(start.cave_idx == end.cave_idx);
		const auto& cave = ECS::get_cave(registry, start);
		if (start.cell_idx >= cave.get_area() || end.cell_idx >= cave.get_area())
			Error::fatal("Find path idx out of bounds");
		if (allow_blocked_end == false && blocks_movement(registry, end))
			return {};
		std::vector<Position> open_set = { start };
		std::map<Position, Position> came_from;
		std::map<Position, double> g_score;
		std::map<Position, double> f_score;

		g_score[start] = 0;
		f_score[start] = cave.distance(start, end);

		while (!open_set.empty())
		{
			Position current_pos = open_set[0];
			for (const Position cell_pos : open_set)
			{	// all open_set elements have f_score mapped
				if (f_score[cell_pos] < f_score[current_pos])
					current_pos = cell_pos;
			}

			if (current_pos == end ||
				(allow_blocked_end == true && cave.distance(current_pos, end) < 1.5))
			{	// found optimal path from start to end
				std::vector<Position> path;
				path.push_back(current_pos);
				while (current_pos != start)
				{	// assign the cell from where we got to to current in backwards order
					current_pos = came_from[current_pos];
					path.push_back(current_pos);
				}
				std::reverse(path.begin(), path.end());
				return path;
			}

			open_set.erase(std::find(open_set.begin(), open_set.end(), current_pos)); // is found
			for (const auto& neighbor_pos : cave.get_nearby_positions(current_pos, 1.5))
			{
				if (!can_move(registry, current_pos, neighbor_pos))
					continue;
				double tentative_g_score = g_score[current_pos] + cave.distance(current_pos, neighbor_pos);
				if (g_score.count(neighbor_pos) == 0)
					g_score[neighbor_pos] = std::numeric_limits<double>::infinity();
				if (tentative_g_score < g_score[neighbor_pos])
				{
					came_from[neighbor_pos] = current_pos;
					g_score[neighbor_pos] = tentative_g_score;
					f_score[neighbor_pos] = tentative_g_score + cave.distance(neighbor_pos, end);
					auto it = std::find(open_set.begin(), open_set.end(), neighbor_pos);
					if (it == open_set.end())
						open_set.push_back(neighbor_pos);
				}
			}
		}
		return {};
	}

	Position get_first_step(const entt::registry& registry, const Position& start, const Position& end)
	{
		if (start == end)
			return start;
		const auto path = find_path(registry, start, end);
		assert(path.size() > 1);
		return path[1];
	}

	/* Does not require entity to have existing Position
	 * Use this when summoning so they will immediately trigger EnterCell Triggers
	 * */
	void move(entt::registry& registry, const entt::entity entity, const Position& position)
	{
		registry.emplace_or_replace<Position>(entity, position);

		Event event = {.type = Event::Type::Move};
		event.actor.entity = entity;
		event.target.position = position;
		ECS::queue_event(registry, event);
	}

};
