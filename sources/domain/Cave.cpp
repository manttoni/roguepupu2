#include <string>          // for basic_string, string
#include <vector>
#include <cmath>
#include <cassert>
#include "utils/Vec2.hpp"
#include "domain/Cave.hpp"        // for Cave
#include "domain/Cell.hpp"        // for Cell

Cave::Cave(const size_t size, const Cell::Type fill) : size(size)
{
	for (size_t i = 0; i < size * size; ++i)
		cells.push_back(Cell(i, fill));
}

std::vector<size_t> Cave::get_nearby_ids(const Cell& middle, const double r, const Cell::Type type) const
{
	return get_nearby_ids(middle.get_idx(), r, type);
}
double Cave::distance(const size_t a, const size_t b) const
{
	assert(a < size && b < size);
	const Vec2 start(a, size);
	const Vec2 end(b, size);
	return std::hypot(start.y - end.y, start.x - end.x);
}

// use with r = 1.5 to get neighbors
std::vector<size_t> Cave::get_nearby_ids(const size_t& middle, const double r, const Cell::Type type) const
{
	std::vector<size_t> neighbors;
	size_t middle_y = middle / size;
	size_t middle_x = middle % size;

	for (int dy = - std::ceil(r); dy <= std::ceil(r); ++dy)
	{
		for (int dx = - std::ceil(r); dx <= std::ceil(r); ++dx)
		{
			if (dy == 0 && dx == 0)
				continue; // middle

			int ny = middle_y + dy;
			int nx = middle_x + dx;

			if (ny < 0 || ny >= static_cast<int>(size) ||
					nx < 0 || nx >= static_cast<int>(size))
				continue;

			size_t nid = ny * size + nx;
			if (distance(middle, nid) > r)
				continue;
			if (type == Cell::Type::None || type == cells[nid].get_type())
				neighbors.push_back(nid);
		}
	}
	return neighbors;
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
		cell.clear_lights();
}
