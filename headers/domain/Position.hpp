#pragma once

#include <climits>
#include <cstdint>

struct Position
{
	static constexpr size_t invalid_idx = SIZE_MAX;
	size_t cell_idx = invalid_idx;
	size_t cave_idx = invalid_idx;

	Position(const size_t cell_idx = invalid_idx,
			const size_t cave_idx = invalid_idx) :
		cell_idx(cell_idx), cave_idx(cave_idx) {}

	static Position invalid_position()
	{
		Position position{};
		assert(!position.is_valid());
		return position;
	}

	bool operator==(const Position& other) const = default;
	bool operator!=(const Position& other) const = default;
	bool operator<(const Position& other) const
	{
		if (cell_idx != other.cell_idx)
			return cell_idx < other.cell_idx;
		return cave_idx < other.cave_idx;
	}

	bool is_valid() const
	{
		return cell_idx != invalid_idx && cave_idx != invalid_idx;
	}
};
