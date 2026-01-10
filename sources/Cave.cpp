#include <stdlib.h>        // for abs
#include <string>          // for basic_string, string
#include "systems/VisionSystem.hpp"
#include "Cave.hpp"        // for Cave
#include "Cell.hpp"        // for Cell
#include "Color.hpp"       // for Color
#include "Components.hpp"  // for Glow, FGColor, Position
#include "Utils.hpp"       // for contains, remove_element, error, Vec2
#include "World.hpp"       // for World
#include "entt.hpp"        // for vector, size_t, map, ceil, allocator, basi...

Cave::Cave(const size_t level, const size_t height, const size_t width, const size_t seed) :
	height(height),
	width(width),
	cells(std::vector<Cell>(height * width)),
	level(level),
	seed(seed),
	humidity(Random::randreal(0, 1, seed))
{
	for (auto& cell : cells)
		cell.set_cave(this);
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
std::vector<size_t> Cave::get_nearby_ids(const size_t& middle, const double r, const Cell::Type type) const
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
			if (type == Cell::Type::None || type == cells[nid].get_type())
				neighbors.push_back(nid);
		}
	}
	return neighbors;
}

std::vector<size_t> Cave::get_empty_cells() const
{
	std::vector<size_t> empty_cells;
	for (const auto& cell : cells)
	{
		if (cell.is_empty())
			empty_cells.push_back(cell.get_idx());
	}
	return empty_cells;
}

std::vector<size_t> Cave::get_cells_with_type(const Cell::Type type) const
{

	std::vector<size_t> cells_with_type;
	for (const auto& cell : cells)
	{
		if (cell.get_type() == type)
			cells_with_type.push_back(cell.get_idx());
	}
	return cells_with_type;
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

void Cave::clear_lights()
{
	for (auto& cell : cells)
	{
		cell.clear_lights();
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
		Color g = color.color * glow.strength;
		cells[ent_idx].add_light(g);
		for (const auto& idx : area)
		{
			if (!VisionSystem::has_vision(registry, entity, cells[idx]))
				continue;

			cells[idx].add_light(g);
		}
	}
}

void Cave::reset_lights()
{
	clear_lights();
	apply_lights();
}


