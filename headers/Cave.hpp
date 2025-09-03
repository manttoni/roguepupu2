#pragma once

#include "Cell.hpp"
#include "Utils.hpp"
#include <string>
#include <vector>
#include <iostream>

class Cave
{
	private:
		size_t height;
		size_t width;
		std::vector<Cell> cells;
		size_t level;
		int seed;

	public:
		/* CONSTRUCTORS */
		Cave();
		Cave(const std::vector<Cell>& cells);
		Cave(size_t height, size_t width);
		Cave(const std::string& map, const size_t width);
		Cave(const Cave &other);

		/* GETTERS */
		size_t get_height() const { return height; }
		size_t get_width() const { return width; }
		size_t get_size() const { return height * width; }
		std::vector<Cell> get_cells() const { return cells; }
		size_t get_level() const { return level; }
		int get_seed() const { return seed; }

		/* SETTERS */
		void set_level(const size_t level) { this->level = level; }
		void set_seed(const int seed) { this->seed = seed; }

		/* TESTING */
		void print_cave(std::vector<std::vector<Cell>> paths = {});

		/* CELL TO CELL */
		std::vector<Cell> find_path(const Cell &start, const Cell &end);
		double distance(const Cell &start, const Cell &end) const;
		double distance(const size_t start_id, const size_t end_id) const;
		std::vector<Cell*> get_neighbors(const Cell &middle);
		std::vector<Cell*> get_nearby_cells(const Cell& middle, const int r);
		bool has_access(const Cell &from, const Cell &to) const;

		/* COUNTING CELLS */
		size_t count_cells(const std::string& type) const;

		/* cave GENERATION */
		std::pair<size_t, size_t> get_stream() const;
};
