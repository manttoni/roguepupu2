#include <stdlib.h>        // for abs
#include <string>          // for basic_string, string
#include "Cave.hpp"        // for Cave
#include "Cell.hpp"        // for Cell
#include "Color.hpp"       // for Color
#include "Components.hpp"  // for Glow, FGColor, Position
#include "Utils.hpp"       // for contains, remove_element, error, Vec2
#include "World.hpp"       // for World
#include "entt.hpp"        // for vector, size_t, map, ceil, allocator, basi...

Cave::Cave() :
	height(0),
	width(0),
	level(0),
	seed(0),
	source(0),
	sink(0) {}

Cave::Cave(const size_t level, const size_t height, const size_t width, const size_t seed) :
	height(height),
	width(width),
	cells(std::vector<Cell>(height * width)),
	level(level),
	seed(seed),
	source(0),
	sink(0)
{
	for (auto& cell : cells)
		cell.set_cave(this);
}

// constructor using premade map
Cave::Cave(const std::string& map, const size_t width) : height(map.size() / width), width(width)
{
	for (size_t i = 0; i < map.size(); ++i)
	{
		Cell::Type type;
		char c = map[i];
		switch (c)
		{
			case 'f':
				type = Cell::Type::FLOOR;
				break;
			case 'w':
				type = Cell::Type::ROCK;
				break;
		}
		cells.push_back(Cell(i, type, this, c));
	}
}

/* CELL TO CELL */
// uses A* to find walkable path from start to end
// start and end can be blocked
std::vector<size_t> Cave::find_path(const size_t start, const size_t end)
{
	if (start >= get_size() || end >= get_size())
		Log::error("Cave::find_path: invalid arguments");
	//if (cells[start].blocks_movement() || cells[end].blocks_movement())
	//	return {}; If this can stay like this, would solve some problems
	std::vector<size_t> open_set = { start };
	std::map<size_t, size_t> came_from;
	std::map<size_t, double> g_score;
	std::map<size_t, double> f_score;

	g_score[start] = 0;
	f_score[start] = distance(start, end);

	while (!open_set.empty())
	{
		size_t current_idx = open_set[0];
		for (const size_t cell_idx : open_set)
		{	// all open_set elements have f_score mapped
			if (f_score[cell_idx] < f_score[current_idx])
				current_idx = cell_idx;
		}

		if (current_idx == end)
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
		for (const size_t neighbor_idx : get_nearby_ids(current_idx, 1.5))
		{
			if (neighbor_idx != end && !has_access(current_idx, neighbor_idx))
				continue;
			double tentative_g_score = g_score[current_idx] + distance(current_idx, neighbor_idx);
			if (g_score.count(neighbor_idx) == 0)
				g_score[neighbor_idx] = std::numeric_limits<double>::infinity();
			if (tentative_g_score < g_score[neighbor_idx])
			{
				came_from[neighbor_idx] = current_idx;
				g_score[neighbor_idx] = tentative_g_score;
				f_score[neighbor_idx] = tentative_g_score + distance(neighbor_idx, end);
				if (!Utils::contains(open_set, neighbor_idx))
					open_set.push_back(neighbor_idx);
			}
		}
	}
	return {};
}

Vec2 Cave::get_direction(const size_t from, const size_t to)
{
	int from_y = from / width;
	int from_x = from % width;
	int to_y = to / width;
	int to_x = to % width;
	return { to_y - from_y, to_x - from_x };
}

double Cave::distance(const Cell &start, const Cell &end) const
{
	size_t start_id = start.get_idx();
	size_t end_id = end.get_idx();
	return distance(start_id, end_id);
}

double Cave::distance(const size_t start_id, const size_t end_id) const
{
	int start_y = start_id / width;
	int start_x = start_id % width;
	int end_y = end_id / width;
	int end_x = end_id % width;

	return std::hypot(start_y - end_y, start_x - end_x);
}

// use with r = 1.5 to get neighbors
std::vector<size_t> Cave::get_nearby_ids(const size_t& middle, const double r) const
{
	std::vector<size_t> neighbors;
	size_t middle_y = middle / width;
	size_t middle_x = middle % width;

	for (int dy = - std::ceil(r); dy <= std::ceil(r); ++dy)
	{
		for (int dx = - std::ceil(r); dx <= std::ceil(r); ++dx)
		{
			if (dy == 0 && dx == 0)
				continue; // middle

			int ny = middle_y + dy;
			int nx = middle_x + dx;

			if (ny < 0 || ny >= static_cast<int>(height) ||
				nx < 0 || nx >= static_cast<int>(width))
				continue;

			size_t nid = ny * width + nx;
			if (distance(middle, nid) > r)
				continue;
			neighbors.push_back(nid);
		}
	}
	if (middle_y != 0 && middle_y != height - 1 && middle_x != 0 && middle_x != width - 1 && r == 1.5)
		assert(neighbors.size() == 8);
	return neighbors;
}

bool Cave::neighbor_has_type(const size_t idx, const Cell::Type type) const
{
	const auto& neighbors = get_nearby_ids(idx, 1.5);
	for (const auto& cell_idx : neighbors)
	{
		const auto& cell = cells[cell_idx];
		if (cell.get_type() == type)
			return true;
	}
	return false;
}

// can someone walk from to. Has to go around corners
bool Cave::has_access(const size_t from_idx, const size_t to_idx) const
{
	if (from_idx >= cells.size() || to_idx >= cells.size())
		return false;
	const auto& to = cells[to_idx];
	if (to.blocks_movement()) // can't move to "to"
		return false;

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

bool Cave::has_vision(const size_t start, const size_t end, const double vision_range) const
{
	if (vision_range > 0 && distance(start, end) > vision_range)
		return false;
	int x0 = static_cast<int>(start % width);
	int y0 = static_cast<int>(start / width);
	int x1 = static_cast<int>(end % width);
	int y1 = static_cast<int>(end / width);

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;

	int err = dx - dy;

	while (true)
	{
		size_t idx = y0 * width + x0;

		if (x0 == x1 && y0 == y1)
			break;
		if (cells[idx].blocks_vision())
			return false;
		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}

	return true;
}

void Cave::clear_lights()
{
	for (auto& cell : cells)
	{
		cell.reset_lights();
	}
}

void Cave::apply_lights()
{
	auto& registry = world->get_registry();
	auto glowing_entities = registry.view<Position, Glow, FGColor>();
	for (const auto& entity : glowing_entities)
	{
		const auto [glow, pos, color] = glowing_entities.get<Glow, Position, FGColor>(entity);

		const auto& ent_idx = pos.cell->get_idx();
		const auto& area = get_nearby_ids(ent_idx, glow.radius);

		for (const auto& idx : area)
		{
			if (!has_vision(ent_idx, idx))
				continue;

			Color g = color.color * glow.strength;
			cells[idx].add_light(g);
		}
	}
}

void Cave::reset_lights()
{
	clear_lights();
	apply_lights();
}


