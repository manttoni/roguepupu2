#include "Area.hpp"
#include "Cell.hpp"
#include "Utils.hpp"
#include <string>
#include <cmath>
#include <cassert>
#include <iostream>

/* CONSTRUCTORS */
// this one randomizes cells then does cellular automata thing
Area::Area(const size_t height, const size_t width) : height(height), width(width)
{
	for (size_t i = 0; i < get_size(); ++i)
	{
		std::string type;
		if (Random::randint(0, 10) < 3)
			type = "wall";
		else
			type = "floor";
		cells.push_back(Cell(i, type));
	}
}

// constructor using premade map
Area::Area(const std::string& map, const size_t width) : height(map.size() / width), width(width)
{
	for (size_t i = 0; i < map.size(); ++i)
	{
		std::string type;
		char c = map[i];
		switch (c)
		{
			case 'f':
				type = "floor";
				break;
			case 'w':
				type = "wall";
				break;
		}
		cells.push_back(Cell(i, type));
	}
}

/* CELL TO CELL */
std::vector<Cell> Area::find_path(const Cell &start, const Cell &end)
{
	if (start.is_blocked())
		return {};
	std::vector<Cell> open_set = {start};
	std::map<Cell, Cell> came_from;

	std::map<Cell, double> g_score;
	std::map<Cell, double> f_score;
	g_score[start] = 0;
	f_score[start] = distance(start, end);

	while (!open_set.empty())
	{
		Cell current = open_set[0];
		for (const Cell &cell : open_set)
		{	// all open_set elements have f_score mapped
			if (f_score[cell] < f_score[current])
				current = cell;
		}

		if (current == end)
		{
			// found optimal path from start to end
			std::vector<Cell> path;
			path.push_back(current);
			while (current != start)
			{	// assign the cell where we got to to current
				current = came_from[current];
				path.push_back(current);
			}
			return path;
		}

		Utils::remove_element(open_set, current);
		for (Cell* neighbor_ptr : get_neighbors(current))
		{
			Cell neighbor = *neighbor_ptr;
			if (!has_access(current, neighbor))
				continue;
			const double tentative_g_score = g_score[current] + distance(current, neighbor);
			if (g_score.count(neighbor) == 0)
				g_score[neighbor] = std::numeric_limits<double>::infinity();
			if (tentative_g_score < g_score[neighbor])
			{
				came_from[neighbor] = current;
				g_score[neighbor] = tentative_g_score;
				f_score[neighbor] = tentative_g_score + distance(neighbor, end);
				if (!Utils::contains(open_set, neighbor))
					open_set.push_back(neighbor);
			}
		}
	}
	return {};
}

void Area::print_area() // for testing
{
	std::vector<Cell> path = find_path(cells[0], cells[get_size() - 1]);
	std::cout << std::string(width * 2 + 1, '=') << std::endl;
	for (size_t i = 0; i < height * width; ++i)
	{
		if (i % width == 0)
			std::cout << "|";
		if (Utils::contains(path, cells[i]))
			std::cout << Colors::GREEN;
		std::cout << cells[i] << Colors::RESET;
		if (i % width == width - 1)
			std::cout << "|" << std::endl;
		else
			std::cout << " ";
	}
	std::cout << std::string(width * 2 + 1, '=') << std::endl;
}

double Area::distance(const Cell &start, const Cell &end) const
{
	size_t start_id = start.get_id();
	size_t end_id = end.get_id();

	size_t start_y = start_id / width;
	size_t start_x = start_id % width;
	size_t end_y = end_id / width;
	size_t end_x = end_id % width;

	return std::hypot(start_y - end_y, start_x - end_x);
}

std::vector<Cell*> Area::get_neighbors(const Cell &middle)
{
	std::vector<Cell*> neighbors;
	size_t middle_id = middle.get_id();
	size_t middle_y = middle_id / width;
	size_t middle_x = middle_id % width;

	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (dy == 0 && dx == 0)
				continue; // middle cell itself

			int ny = middle_y + dy;
			int nx = middle_x + dx;
			if (ny < 0 || ny >= static_cast<int>(height) ||
				nx < 0 || nx >= static_cast<int>(width))
				continue; // out of bounds

			size_t nid = ny * width + nx;
			neighbors.push_back(&cells[nid]);
		}
	}

	if (width == 25 && height == 25)
	{
		if (middle_id == 0)
			assert(neighbors.size() == 3);
		if (middle_id == 25)
			assert(neighbors.size() == 5);
		if (middle_id == 26)
			assert(neighbors.size() == 8);
	}

	return neighbors;
}

bool Area::has_access(const Cell &from, const Cell &to) const
{
	if (to.is_blocked()) // can't move to "to"
		return false;

	size_t fy = from.get_id() / width;
	size_t fx = from.get_id() % width;
	size_t ty = to.get_id() / width;
	size_t tx = to.get_id() % width;

	if (abs(fy - ty) > 1 || abs(fx - tx) > 1) // is not a neighbor
		return false;

	// there is access from "from" to "to" if they are on same x or y axis
	if (fy == ty || fx == tx)
		return true;

	// there is access diagonally if there is no corner to go around
	const Cell corner1 = cells[fy * width + tx];
	const Cell corner2 = cells[ty * width + fx];
	if (corner1.is_blocked() || corner2.is_blocked())
		return false;
	return true;
}
