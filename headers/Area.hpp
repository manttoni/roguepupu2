#pragma once

#include "Cell.hpp"
#include "Utils.hpp"
#include <string>
#include <vector>
#include <iostream>

class Area
{
	private:
		std::string name;
		size_t height;
		size_t width;
		std::vector<Cell> cells;

	public:
		/* CONSTRUCTORS */
		Area();
		Area(const std::string &name, size_t height, size_t width);
		Area(const Area &other);

		/* GETTERS */
		std::string get_name() const { return name; }
		size_t get_height() const { return height; }
		size_t get_width() const { return width; }
		size_t get_size() const { return height * width; }

		/* TESTING */
		void print_area() const;

		/* CELL TO CELL */
		std::vector<Cell> find_path(const Cell &start, const Cell &end) const;
		double distance(const Cell &start, const Cell &end) const;
		std::vector<Cell> get_neighbors(const Cell &middle) const;
		bool has_access(const Cell &from, const Cell &to) const;
};
