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
		size_t depth;
	public:
		size_t get_depth() const { return depth; }

	private:
		std::vector<size_t> connections;
	public:
		std::vector<size_t> get_connections() const { return connections; }
		void set_connections(const std::vector<size_t> connections) { this->connections = connections; }
		void add_connection(const size_t connection) { connections.push_back(connection); }

	private:
		std::vector<Cell> cells;
	public:
		const std::vector<Cell>& get_cells() const { return cells; }
		std::vector<Cell>& get_cells() { return cells; }

		std::vector<Position> get_positions() const;

		const Cell& get_cell(const size_t idx) const { return cells[idx]; }
		Cell& get_cell(const size_t idx) { return cells[idx]; }

		bool validate_position(const Position& position) const
		{
			if (!position.is_valid())
				Log::debug() << position << " is not valid";
			if (position.cave_idx != idx)
				Log::debug() << position << " is not in this cave";
			if (position.cell_idx >= get_area())
				Log::debug() << position << " is out of bounds";
			assert(position.is_valid());
			assert(position.cave_idx == idx);
			assert(position.cell_idx < get_area());
			return true;
		}
		const Cell& get_cell(const Position& position) const
		{
			validate_position(position);
			return cells[position.cell_idx];
		}
		Cell& get_cell(const Position& position)
		{
			validate_position(position);
			return cells[position.cell_idx];
		}

		std::vector<Position> get_nearby_positions(const size_t middle, const double r = 1.5, const Cell::Type type = Cell::Type::None) const;
		std::vector<Position> get_nearby_positions(const Position& middle, const double r = 1.5, const Cell::Type type = Cell::Type::None) const;
		std::vector<Position> get_positions_with_type(const Cell::Type type) const;

		Position middle_position() const { return Position((size / 2) * size + (size / 2), idx); }
		Position deepest_position() const;

	public:
		Cave(const size_t idx, const size_t size, const size_t depth, const Cell::Type fill = Cell::Type::Rock);
		Cave(const Cave& other) = default;
		Cave& operator=(const Cave& other) = default;

		double distance(const Position& a, const Position& b) const;
		double distance(const size_t a, const size_t b) const;
		void clear_lights();
		Vec2<double> flow_direction(const size_t cell_idx) const;
		Vec2<double> get_direction(const size_t from, const size_t to) const;
};

inline std::ostream& operator<<(std::ostream& os, const Cave& cave)
{
	os << "idx: " << cave.get_idx() << " | size: " << cave.get_size() << " | depth: " << cave.get_depth() << " | connections: " << cave.get_connections().size();
	return os;
}
