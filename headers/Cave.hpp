#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "Cell.hpp"
#include "Utils.hpp"

class Cave
{
	private:
		size_t height, width;
		std::vector<Cell> cells;
		size_t level;
		size_t seed;
		size_t source, sink; // water flow

	public:
		/* CONSTRUCTORS */
		Cave();
		Cave(const size_t level, const size_t height, const size_t width, const size_t seed);
		Cave(const std::string& map, const size_t width);
		Cave(const Cave &other);
		Cave& operator=(const Cave& other);

		/* GETTERS */
		size_t get_height() const { return height; }
		size_t get_width() const { return width; }
		size_t get_size() const { return height * width; }
		const std::vector<Cell>& get_cells() const { return cells; }
		std::vector<Cell>& get_cells() { return cells; }
		size_t get_level() const { return level; }
		int get_seed() const { return seed; }
		size_t get_source() const { return source; }
		size_t get_sink() const { return sink; }

		/* SETTERS */
		void set_level(const size_t level) { this->level = level; }
		void set_seed(const int seed) { this->seed = seed; }
		void set_cells(const std::vector<Cell>& cells) { this->cells = cells; }
		void set_source(const size_t source)
		{
			this->source = source;
			cells[source].set_type(Cell::Type::SOURCE);
		}
		void set_sink(const size_t sink)
		{
			this->sink = sink;
			cells[sink].set_type(Cell::Type::SINK);
		}

		/* TESTING */
		void print_cave();

		/* CELL TO CELL */
		std::vector<size_t> find_path(const size_t start, const size_t end);
		double distance(const Cell &start, const Cell &end) const;
		double distance(const size_t start_id, const size_t end_id) const;
		std::vector<size_t> get_nearby_ids(const size_t& middle, const double r) const;
		bool has_access(const size_t from_idx, const size_t to_idx) const;
		bool neighbor_has_type(const size_t middle, const Cell::Type type) const;
		bool has_vision(const size_t from, const size_t to) const;

		void reset_effects();
};
