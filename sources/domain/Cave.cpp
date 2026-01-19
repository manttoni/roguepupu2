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

std::vector<Position> Cave::get_positions() const
{
	std::vector<Position> positions;
	for (size_t i = 0; i < size * size; ++i)
		positions.push_back(Position(i, idx));
	return positions;
}

double Cave::distance(const Position& a, const Position& b) const
{
	assert(a.is_valid());
	assert(b.is_valid());
	assert(a.cave_idx == idx && b.cave_idx == idx);
	assert(a.cell_idx < get_area() && b.cell_idx < get_area());
	const Vec2 start(a.cell_idx, size);
	const Vec2 end(b.cell_idx, size);
	return std::hypot(start.y - end.y, start.x - end.x);
}

double Cave::distance(const size_t a, const size_t b) const
{
	assert(a < get_size() && b < get_size());
	const Vec2 start(a, size);
	const Vec2 end(b, size);
	return std::hypot(start.y - end.y, start.x - end.x);
}

std::vector<Position> Cave::get_nearby_positions(const Position& middle_pos, const double r, const Cell::Type type) const
{
	assert(middle_pos.is_valid());
	assert(middle_pos.cave_idx == idx);
	assert(middle_pos.cell_idx < get_area());
	std::vector<Position> neighbors;
	const Vec2 middle(middle_pos.cell_idx, size);

	for (int dy = - std::ceil(r); dy <= std::ceil(r); ++dy)
	{
		for (int dx = - std::ceil(r); dx <= std::ceil(r); ++dx)
		{
			if (dy == 0 && dx == 0)
				continue; // middle

			int ny = middle.y + dy;
			int nx = middle.x + dx;

			if (ny < 0 || ny >= static_cast<int>(size) ||
					nx < 0 || nx >= static_cast<int>(size))
				continue;

			const size_t neighbor_idx = ny * size + nx;
			const Position neighbor_pos(neighbor_idx, idx);
			if (distance(middle_pos, neighbor_pos) > r)
				continue;
			if (type == Cell::Type::None || type == get_cell(neighbor_pos).get_type())
				neighbors.push_back(neighbor_pos);
		}
	}
	return neighbors;
}

std::vector<Position> Cave::get_positions_with_type(const Cell::Type type) const
{
	std::vector<Position> positions;
	for (const auto& cell : cells)
	{
		if (cell.get_type() == type)
			positions.push_back(Position(cell.get_idx(), idx));
	}
	return positions;
}

void Cave::clear_lights()
{
	for (auto& cell : cells)
		cell.clear_lights();
}
