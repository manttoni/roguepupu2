#pragma once

#include <vector>
#include <string>     // for string
#include "Cell.hpp"   // for Cell
#include "Color.hpp"  // for Color

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

		const Cell& get_cell(const size_t idx) const { return cells[idx]; }
		Cell& get_cell(const size_t idx) { return cells[idx]; }

		std::vector<size_t> get_empty_cells() const;
		std::vector<size_t> get_cells_with_type(const Cell::Type type) const;

	public:
		Cave(const size_t size, const Cell::Type fill = Cell::Type::Rock);

		Cave(const Cave& other) = default;
		Cave& operator=(const Cave& other) = default;
		Cave(Cave&& other) = default;
		Cave& operator=(Cave&& other) = default;

		std::vector<size_t> get_nearby_ids(const Cell& middle, const double r = 1.5, const Cell::Type type = Cell::Type::None) const;
		std::vector<size_t> get_nearby_ids(const size_t& middle, const double r = 1.5, const Cell::Type type = Cell::Type::None) const;
		bool neighbor_has_type(const size_t middle, const Cell::Type type) const;
		double distance(const size_t a, const size_t b) const;
		void clear_lights();
};
