#include "domain/Cave.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

#include "utils/Log.hpp"
#include "utils/Math.hpp"
#include "utils/Vec2.hpp"

Cave::Cave(
	const size_t idx,
	const size_t size,
	const size_t depth,
	const Cell::Type fill
)
	: idx(idx),
	  size(size),
	  depth(depth)
{
	cells.reserve(get_area());

	for (size_t i = 0; i < get_area(); ++i)
	{
		cells.emplace_back(i, fill);

		auto& cell = cells.back();

		if (cell.get_type() == Cell::Type::Floor)
			cell.set_glyph('.');
	}
}

size_t Cave::get_idx() const noexcept
{
	return idx;
}

void Cave::set_idx(const size_t new_idx) noexcept
{
	idx = new_idx;
}

size_t Cave::get_size() const noexcept
{
	return size;
}

size_t Cave::get_area() const noexcept
{
	return size * size;
}

size_t Cave::get_depth() const noexcept
{
	return depth;
}

const std::vector<size_t>& Cave::get_connections() const noexcept
{
	return connections;
}

void Cave::set_connections(std::vector<size_t> new_connections)
{
	connections = std::move(new_connections);
}

void Cave::add_connection(const size_t connection)
{
	connections.push_back(connection);
}

const std::vector<Cell>& Cave::get_cells() const noexcept
{
	return cells;
}

std::vector<Cell>& Cave::get_cells() noexcept
{
	return cells;
}

const Cell& Cave::get_cell(const size_t cell_idx) const
{
	return cells.at(cell_idx);
}

Cell& Cave::get_cell(const size_t cell_idx)
{
	return cells.at(cell_idx);
}

const Cell& Cave::get_cell(const Position& position) const
{
	validate_position(position);
	return cells[position.cell_idx];
}

Cell& Cave::get_cell(const Position& position)
{
	validate_position(position);
	return cells[position.cell_idx];
}

void Cave::validate_position(const Position& position) const
{
	if (!position.is_valid())
		Log::debug() << position << " is not valid";

	if (position.cave_idx != idx)
		Log::debug() << position << " is not in cave " << idx;

	if (position.cell_idx >= get_area())
		Log::debug() << position << " is out of bounds";

	assert(position.is_valid());
	assert(position.cave_idx == idx);
	assert(position.cell_idx < get_area());
}

std::vector<Position> Cave::get_positions() const
{
	std::vector<Position> positions;
	positions.reserve(get_area());

	for (size_t cell_idx = 0; cell_idx < get_area(); ++cell_idx)
		positions.emplace_back(cell_idx, idx);

	return positions;
}

std::vector<Position> Cave::get_nearby_positions(
	const size_t middle,
	const double radius,
	const Cell::Type type
) const
{
	return get_nearby_positions(Position{middle, idx}, radius, type);
}

std::vector<Position> Cave::get_nearby_positions(
	const Position& middle_position,
	const double radius,
	const Cell::Type type
) const
{
	validate_position(middle_position);

	std::vector<Position> neighbors;

	const auto middle =
		Vec2<int>::from_idx(middle_position.cell_idx, size);

	const int search_radius =
		static_cast<int>(std::ceil(radius));

	for (int dy = -search_radius; dy <= search_radius; ++dy)
	{
		for (int dx = -search_radius; dx <= search_radius; ++dx)
		{
			if (dy == 0 && dx == 0)
				continue;

			const int y = middle.y + dy;
			const int x = middle.x + dx;

			if (
				y < 0 ||
				y >= static_cast<int>(size) ||
				x < 0 ||
				x >= static_cast<int>(size)
			)
			{
				continue;
			}

			const size_t neighbor_idx =
				static_cast<size_t>(y) * size +
				static_cast<size_t>(x);

			const Position neighbor_position{
				neighbor_idx,
				idx
			};

			if (distance(middle_position, neighbor_position) > radius)
				continue;

			if (
				type != Cell::Type::None &&
				get_cell(neighbor_position).get_type() != type
			)
			{
				continue;
			}

			neighbors.push_back(neighbor_position);
		}
	}

	return neighbors;
}

std::vector<Position> Cave::get_positions_with_type(
	const Cell::Type type
) const
{
	std::vector<Position> positions;

	for (const auto& cell : cells)
	{
		if (cell.get_type() == type)
			positions.emplace_back(cell.get_idx(), idx);
	}

	return positions;
}

Position Cave::middle_position() const
{
	const size_t middle = size / 2;

	return Position{
		middle * size + middle,
		idx
	};
}

Position Cave::deepest_position() const
{
	if (cells.empty())
		return Position{};

	size_t deepest_cell_idx = 0;
	double deepest_density =
		cells.front().get_effective_density();

	for (size_t i = 1; i < cells.size(); ++i)
	{
		const double density =
			cells[i].get_effective_density();

		if (density < deepest_density)
		{
			deepest_density = density;
			deepest_cell_idx = i;
		}
	}

	return Position{
		deepest_cell_idx,
		idx
	};
}

double Cave::distance(
	const Position& a,
	const Position& b
) const
{
	validate_position(a);
	validate_position(b);

	return distance(a.cell_idx, b.cell_idx);
}

double Cave::distance(
	const size_t a,
	const size_t b
) const
{
	assert(a < get_area());
	assert(b < get_area());

	const auto start = Vec2<int>::from_idx(a, size);
	const auto end = Vec2<int>::from_idx(b, size);

	return std::hypot(
		start.y - end.y,
		start.x - end.x
	);
}

bool Cave::contains(const Position& position) const
{
	return contains(position.cell_idx);
}

bool Cave::contains(const Vec2<int>& vec) const
{
	return vec.y >= 0 && vec.y < static_cast<int>(size)
		&& vec.x >= 0 && vec.y < static_cast<int>(size);
}

bool Cave::contains(const size_t idx) const
{
	return idx < size;
}

Vec2<double> Cave::get_direction(
	const size_t from,
	const size_t to
) const
{
	assert(from < get_area());
	assert(to < get_area());
	assert(from != to);

	const auto from_position =
		Vec2<double>::from_idx(from, size);

	const auto to_position =
		Vec2<double>::from_idx(to, size);

	const auto direction =
		to_position - from_position;

	assert(std::isfinite(direction.y));
	assert(std::isfinite(direction.x));

	return direction;
}

std::ostream& operator<<(
	std::ostream& os,
	const Cave& cave
)
{
	return os
		<< "idx: " << cave.get_idx()
		<< " | size: " << cave.get_size()
		<< " | depth: " << cave.get_depth()
		<< " | connections: "
		<< cave.get_connections().size();
}

Position Cave::offset_position(const Position& position, const Vec2<int>& offset) const
{
	const auto pv2 = Vec2<int>::from_idx(position.cell_idx, size);
	const auto tv2 = pv2 + offset;
	if (tv2.x < 0 || tv2.y < 0 || tv2.x >= static_cast<int>(size) || tv2.y >= static_cast<int>(size))
		return Position::invalid_position();
	return Position(tv2.to_idx(size), idx);
}
