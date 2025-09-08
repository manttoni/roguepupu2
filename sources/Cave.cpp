#include <string>
#include <cmath>
#include <cassert>
#include <iostream>
#include <ncurses.h>
#include "Cave.hpp"
#include "Cell.hpp"
#include "Utils.hpp"
#include "UI.hpp"

/* CONSTRUCTORS */
Cave::Cave()
	: height(0), width(0), cells({}), level(0), seed(0), water_start(0), water_end(0) {}
Cave::Cave(const size_t height, const size_t width, const std::vector<Cell>& cells, const size_t level, const size_t seed)
	: height(height), width(width), cells(cells), level(level), seed(seed), water_start(0), water_end(0) {}
Cave::Cave(const Cave& other)
	: height(other.height), width(other.width), cells(other.cells), level(other.level), seed(other.seed), water_start(other.water_start), water_end(other.water_end) {}

Cave& Cave::operator=(const Cave& other)
{
	if (this == &other)
		return *this;

	height = other.height;
	width = other.width;
	cells = other.cells;
	level = other.level;
	seed = other.seed;
	water_start = other.water_start;
	water_end = other.water_end;
	return *this;
}

// constructor using premade map
Cave::Cave(const std::string& map, const size_t width) : height(map.size() / width), width(width)
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
				type = "rock";
				break;
		}
		cells.push_back(Cell(i, type));
	}
}

/* CELL TO CELL */
// uses A* to find walkable path from start to end
std::vector<Cell> Cave::find_path(const Cell &start, const Cell &end)
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
		{	// found optimal path from start to end
			std::vector<Cell> path;
			path.push_back(current);
			while (current != start)
			{	// assign the cell from where we got to to current
				current = came_from[current];
				path.push_back(current);
			}
			return path;
		}

		Utils::remove_element(open_set, current);
		for (Cell* neighbor : get_neighbors(current))
		{
			if (!has_access(current, *neighbor))
				continue;
			double tentative_g_score = g_score[current] + distance(current, *neighbor);
			if (g_score.count(*neighbor) == 0)
				g_score[*neighbor] = std::numeric_limits<double>::infinity();
			if (tentative_g_score < g_score[*neighbor])
			{
				came_from[*neighbor] = current;
				g_score[*neighbor] = tentative_g_score;
				f_score[*neighbor] = tentative_g_score + distance(*neighbor, end);
				if (!Utils::contains(open_set, *neighbor))
					open_set.push_back(*neighbor);
			}
		}
	}
	return {};
}

double Cave::distance(const Cell &start, const Cell &end) const
{
	size_t start_id = start.get_id();
	size_t end_id = end.get_id();
	return distance(start_id, end_id);
}

double Cave::distance(const size_t start_id, const size_t end_id) const
{
	int start_y = start_id / width;
	int start_x = start_id % width;
	int end_y = end_id / width;
	int end_x = end_id % width;

	return std::hypot(start_y - end_y, start_x - end_x);
}

// return all cells within a radius
std::vector<Cell*> Cave::get_nearby_cells(const Cell &middle, const int r)
{
	std::vector<Cell*> nearby;
	size_t middle_id = middle.get_id();
	int middle_y = middle_id / width;
	int middle_x = middle_id % width;

	for (int dy = -r; dy <= r; ++dy)
	{
		for (int dx = -r; dx <= r; ++dx)
		{
			int ny = middle_y + dy;
			int nx = middle_x + dx;
			if (ny < 0 || ny >= static_cast<int>(height) ||
				nx < 0 || nx >= static_cast<int>(width))
				continue; // out of bounds
						  //
			size_t nid = ny * width + nx;
			if (distance(middle, cells[nid]) > r)
				continue;
			nearby.push_back(&cells[nid]);
		}
	}

	return nearby;
}

// return adjacent cells, also diagonal
std::vector<Cell*> Cave::get_neighbors(const Cell &middle)
{
	assert(middle.get_id() < get_size());
	std::vector<Cell*> neighbors;
	size_t middle_id = middle.get_id();
	size_t middle_y = middle_id / width;
	size_t middle_x = middle_id % width;
	assert(middle_y < height && middle_x < width);

	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (dy == 0 && dx == 0)
				continue; // middle cell itself

			int ny = middle_y + dy;
			int nx = middle_x + dx;
			assert(ny < static_cast<int>(height) + 1 && nx < static_cast<int>(width) + 1);
			if (ny < 0 || ny >= static_cast<int>(height) ||
				nx < 0 || nx >= static_cast<int>(width))
				continue; // out of bounds

			size_t nid = ny * width + nx;
			neighbors.push_back(&cells[nid]);
			assert(distance(middle, cells[nid]) < 2);
		}
	}
	return neighbors;
}

// can someone walk from to. Has to go around corners
bool Cave::has_access(const Cell &from, const Cell &to) const
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

