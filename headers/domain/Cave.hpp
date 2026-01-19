#pragma once

#include <cassert>
#include <vector>
#include <string>     // for string
#include "domain/Cell.hpp"   // for Cell
#include "domain/Position.hpp"

class Cave
{
	private:
		size_t idx;
	public:
		size_t get_idx() const { return idx; }
		void set_idx(const size_t idx) { this->idx = idx; }

	private:
		size_t size;
	public:
		size_t get_size() const { return size; }
		size_t get_area() const { return size * size; }

	private:
		std::vector<Cell> cells;
	public:
		const std::vector<Cell>& get_cells() const { return cells; }
		std::vector<Cell>& get_cells() { return cells; }

		std::vector<Position> get_positions() const;

		const Cell& get_cell(const Position& position) const
		{
			assert(position.is_valid());
			assert(position.cave_idx == idx);
			assert(position.cell_idx < get_area());
			return cells[position.cell_idx];
		}
		Cell& get_cell(const Position& position)
		{
			assert(position.is_valid());
			assert(position.cave_idx == idx);
			assert(position.cell_idx < get_area());
			return cells[position.cell_idx];
		}

		std::vector<Position> get_nearby_positions(const Position& middle, const double r = 1.5, const Cell::Type type = Cell::Type::None) const;
		std::vector<Position> get_positions_with_type(const Cell::Type type) const;

	public:
		Cave(const size_t size, const Cell::Type fill = Cell::Type::Rock);

		Cave(const Cave& other) = default;
		Cave& operator=(const Cave& other) = default;

		double distance(const Position& a, const Position& b) const;
		double distance(const size_t a, const size_t b) const;
		void clear_lights();
};
