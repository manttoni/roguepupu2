#pragma once

#include <climits>
#include <cstdint>

struct Position
{
	static constexpr size_t invalid_id = SIZE_MAX;
	size_t cell_idx = invalid_id;
	size_t cave_idx = invalid_id;

	Position(const size_t cell_idx = invalid_id,
			const size_t cave_idx = invalid_id) :
		cell_idx(cell_idx), cave_idx(cave_idx) {}

	bool operator==(const Position& other) const = default;
	bool operator!=(const Position& other) const = default;
};
