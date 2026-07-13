#include <stddef.h>
#include <vector>
#include <cmath>
#include <cassert>

#include "utils/Vec2.hpp"
#include "utils/Math.hpp"
#include "utils/Random.hpp"
#include "domain/Cave.hpp"        // for Cave
#include "domain/Cell.hpp"        // for Cell
#include "domain/Position.hpp"

Cave::Cave(const size_t idx, const size_t size, const size_t depth, const Cell::Type fill)
	: idx(idx), size(size), depth(depth)
{
	//static const std::string floor_glyphs = " ,.:;\'\"";
	for (size_t i = 0; i < get_area(); ++i)
	{
		cells.emplace_back(i, fill);
		auto& cell = cells.back();
		if (cell.get_type() == Cell::Type::Floor)
		{
			cells.back().set_glyph('.'); // Plan is to have some floor glyph system at some point,
										 // RenderingSystem might handle that. Some system that will
										 // also update the glyph if it should change because of
										 // density(which represents elevation when negative) or some other reason
		}
	}
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
	const Vec2<int> start = Vec2<int>::from_idx(a.cell_idx, size);
	const Vec2<int> end = Vec2<int>::from_idx(b.cell_idx, size);
	return std::hypot(start.y - end.y, start.x - end.x);
}

double Cave::distance(const size_t a, const size_t b) const
{
	assert(a < get_size() && b < get_size());
	const Vec2<int> start = Vec2<int>::from_idx(a, size);
	const Vec2<int> end = Vec2<int>::from_idx(b, size);
	return std::hypot(start.y - end.y, start.x - end.x);
}

std::vector<Position> Cave::get_nearby_positions(const size_t middle, const double r, const Cell::Type type) const
{
	return get_nearby_positions(Position(middle, idx), r, type);
}

std::vector<Position> Cave::get_nearby_positions(const Position& middle_pos, const double r, const Cell::Type type) const
{
	validate_position(middle_pos);
	std::vector<Position> neighbors;
	const Vec2<int> middle = Vec2<int>::from_idx(middle_pos.cell_idx, size);

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

/* Return the direction liquids will flow
 * */
Vec2<double> Cave::flow_direction(const size_t cell_idx) const
{
	Log::debug() << "flow_direction(" << cell_idx << ")";
	Vec2<double> flow{0.0, 0.0};

	assert(!std::isnan(flow.y));
	assert(!std::isnan(flow.x));

	const auto& cell = get_cell(cell_idx);

	size_t debug_i = 0;

	for (const auto& n : get_nearby_positions(cell_idx, 1.5, Cell::Type::Floor))
	{
		assert(n.is_valid());
		assert(n.cell_idx < get_area());
		const auto& neighbor = get_cell(n);

		const auto cell_density = Math::clamp<double>(cell.get_density(), -1000, 1000);
		const auto neig_density = Math::clamp<double>(neighbor.get_density(), -1000, 1000);
		const double steepness = cell_density - neig_density;
		assert(!std::isnan(cell.get_density()));
		assert(!std::isnan(neighbor.get_density()));
		assert(!std::isnan(steepness));

		if (steepness <= 0.0)
			continue;

		const auto dir = get_direction(cell_idx, n.cell_idx);

		assert(!std::isnan(dir.y));
		assert(!std::isnan(dir.x));

		flow += dir * steepness;

		/* EXPLODES HERE FIRST ITERATION */
		assert(!std::isnan(flow.y));
		assert(!std::isnan(flow.x));

		assert(std::isfinite(flow.y));
		assert(std::isfinite(flow.x));
		/**/

		Log::debug() << debug_i++;
	}
	return flow;
}

/* Returns the direction where the other cell is
 * */
Vec2<double> Cave::get_direction(const size_t from, const size_t to) const
{
	assert(from != to);
	const auto from_vec = Vec2<double>::from_idx(from, size);
	const auto to_vec = Vec2<double>::from_idx(to, size);
	const auto diff = from_vec - to_vec;
	assert(!std::isnan(diff.y));
	assert(!std::isnan(diff.x));
	const auto normal = diff.normalize();
	assert(!std::isnan(diff.y));
	assert(!std::isnan(diff.x));
	assert(!std::isnan(normal.y));
	assert(!std::isnan(normal.x));
	//return normal; no reason to normalize maybe?
	return diff;
}

Position Cave::deepest_position() const
{
	double deepest = 0;
	size_t cell_idx = 0;
	for (size_t i = 0; i < get_area(); ++i)
	{
		const auto ed = cells[i].get_effective_density();
		if (ed < deepest)
		{
			deepest = ed;
			cell_idx = i;
		}
	}
	return Position{cell_idx, idx};
}
