#include <string>
#include <cmath>
#include <cassert>
#include <iostream>
#include <ncurses.h>
#include "Cave.hpp"
#include "Cell.hpp"
#include "Utils.hpp"
#include "UI.hpp"
#include "Light.hpp"
#include "Creature.hpp"

/* CONSTRUCTORS */
// DEFAULT
Cave::Cave() :
	height(0),
	width(0),
	level(0),
	seed(0),
	source(0),
	sink(0) {}

// CANVAS IN CAVEGENERATOR
Cave::Cave(const size_t level, const size_t height, const size_t width, const size_t seed) :
	height(height),
	width(width),
	cells(std::vector<Cell>(height * width)),
	level(level),
	seed(seed),
	source(0),
	sink(0)
{}

// COPY
Cave::Cave(Cave&& other) :
	height(other.height),
	width(other.width),
	cells(std::move(other.cells)),
	level(other.level),
	seed(other.seed),
	source(other.source),
	sink(other.sink) {}

Cave& Cave::operator=(Cave&& other)
{
	if (this == &other)
		return *this;

	height = other.height;
	width = other.width;
	cells = std::move(other.cells);
	level = other.level;
	seed = other.seed;
	source = other.source;
	sink = other.sink;
	return *this;
}

// constructor using premade map
Cave::Cave(const std::string& map, const size_t width) : height(map.size() / width), width(width)
{
	for (size_t i = 0; i < map.size(); ++i)
	{
		Cell::Type type;
		char c = map[i];
		switch (c)
		{
			case 'f':
				type = Cell::Type::FLOOR;
				break;
			case 'w':
				type = Cell::Type::ROCK;
				break;
		}
		cells.push_back(Cell(i, type, this, c));
	}
}

/* CELL TO CELL */
// uses A* to find walkable path from start to end
std::vector<size_t> Cave::find_path(const size_t start, const size_t end)
{
	if (start >= get_size() || end >= get_size())
		throw std::runtime_error("Cave::find_path: invalid arguments");
	if (cells[start].blocks_movement())
		return {};
	std::vector<size_t> open_set = { start };
	std::map<size_t, size_t> came_from;
	std::map<size_t, double> g_score;
	std::map<size_t, double> f_score;

	g_score[start] = 0;
	f_score[start] = distance(start, end);

	while (!open_set.empty())
	{
		size_t current_idx = open_set[0];
		for (const size_t cell_idx : open_set)
		{	// all open_set elements have f_score mapped
			if (f_score[cell_idx] < f_score[current_idx])
				current_idx = cell_idx;
		}

		if (current_idx == end)
		{	// found optimal path from start to end
			std::vector<size_t> path;
			path.push_back(current_idx);
			while (current_idx != start)
			{	// assign the cell from where we got to to current
				current_idx = came_from[current_idx];
				path.push_back(current_idx);
			}
			return path;
		}

		Utils::remove_element(open_set, current_idx);
		for (const size_t neighbor_idx : get_nearby_ids(current_idx, 1.5))
		{
			if (!has_access(current_idx, neighbor_idx))
				continue;
			double tentative_g_score = g_score[current_idx] + distance(current_idx, neighbor_idx);
			if (g_score.count(neighbor_idx) == 0)
				g_score[neighbor_idx] = std::numeric_limits<double>::infinity();
			if (tentative_g_score < g_score[neighbor_idx])
			{
				came_from[neighbor_idx] = current_idx;
				g_score[neighbor_idx] = tentative_g_score;
				f_score[neighbor_idx] = tentative_g_score + distance(neighbor_idx, end);
				if (!Utils::contains(open_set, neighbor_idx))
					open_set.push_back(neighbor_idx);
			}
		}
	}
	return {};
}

double Cave::distance(const Cell &start, const Cell &end) const
{
	size_t start_id = start.get_idx();
	size_t end_id = end.get_idx();
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

// use with r = 1.5 to get neighbors
std::vector<size_t> Cave::get_nearby_ids(const size_t& middle, const double r) const
{
	std::vector<size_t> neighbors;
	size_t middle_y = middle / width;
	size_t middle_x = middle % width;

	for (int dy = - std::ceil(r); dy <= std::ceil(r); ++dy)
	{
		for (int dx = - std::ceil(r); dx <= std::ceil(r); ++dx)
		{
			if (dy == 0 && dx == 0)
				continue; // middle

			int ny = middle_y + dy;
			int nx = middle_x + dx;

			if (ny < 0 || ny >= static_cast<int>(height) ||
				nx < 0 || nx >= static_cast<int>(width))
				continue;

			size_t nid = ny * width + nx;
			if (distance(middle, nid) > r)
				continue;
			neighbors.push_back(nid);
		}
	}
	if (middle_y != 0 && middle_y != height - 1 && middle_x != 0 && middle_x != width - 1 && r == 1.5)
		assert(neighbors.size() == 8);
	return neighbors;
}

bool Cave::neighbor_has_type(const size_t idx, const Cell::Type type) const
{
	const auto& neighbors = get_nearby_ids(idx, 1.5);
	for (const auto& cell_idx : neighbors)
	{
		const auto& cell = cells[cell_idx];
		if (cell.get_type() == type)
			return true;
	}
	return false;
}

// can someone walk from to. Has to go around corners
bool Cave::has_access(const size_t from_idx, const size_t to_idx) const
{
	const auto& to = cells[to_idx];
	if (to.blocks_movement()) // can't move to "to"
		return false;

	size_t fy = from_idx / width;
	size_t fx = from_idx % width;
	size_t ty = to_idx / width;
	size_t tx = to_idx % width;

	if (abs(fy - ty) > 1 || abs(fx - tx) > 1) // is not a neighbor
		return false;

	// there is access from "from" to "to" if they are on same x or y axis
	if (fy == ty || fx == tx)
		return true;

	// there is access diagonally if there is no corner to go around
	const auto& corner1 = cells[fy * width + tx];
	const auto& corner2 = cells[ty * width + fx];
	if (corner1.blocks_movement() || corner2.blocks_movement())
		return false;
	return true;
}

void Cave::clear_lights()
{
	for (auto& cell : cells)
	{
		cell.reset_lights();
	}
}

void Cave::apply_lights()
{
	for (auto& cell : cells)
	{
		for (const auto& entity : cell.get_entities())
		{
			for (auto& light : entity->get_lights())
			{
				light.trigger(*this, cell.get_idx());
			}
		}
	}
}

void Cave::reset_lights()
{
	clear_lights();
	apply_lights();
}

bool Cave::has_vision(size_t start, size_t end) const
{
	int x0 = static_cast<int>(start % width);
	int y0 = static_cast<int>(start / width);
	int x1 = static_cast<int>(end % width);
	int y1 = static_cast<int>(end / width);

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;

	int err = dx - dy;

	while (true)
	{
		size_t idx = y0 * width + x0;

		if (x0 == x1 && y0 == y1)
			break;
		if (cells[idx].blocks_vision())
			return false;
		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}

	return true;
}

// draw cave from perspective of player or other creature
void Cave::draw(const Creature& player)
{
	Log::log("Drawing cave");
	const size_t player_idx = player.get_idx();
	reset_lights();

	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	WINDOW* window = panel_window(panel);
	werase(window);

	UI::instance().set_current_panel(panel);

	int window_height, window_width;
	getmaxyx(window, window_height, window_width);

	size_t y_player = player_idx / width;
	size_t x_player = player_idx % width;

	size_t y_center = window_height / 2;
	size_t x_center = window_width / 2;

	auto cells_in_vision_range = get_nearby_ids(player_idx, player.get_vision_range());
	cells_in_vision_range.push_back(player_idx);

	for (const auto& cell_idx : cells_in_vision_range)
	{
		size_t y_cell = cell_idx / width;
		size_t x_cell = cell_idx % width;

		int y = y_center + y_cell - y_player;
		int x = x_center + x_cell - x_player;
		if (y < 0 || y >= window_height || x < 0 || x >= window_width)
			continue;
		if (!has_vision(player_idx, cell_idx))
			continue;

		const auto& cell = cells[cell_idx];
		const auto& color_pair = cell.get_color_pair();
		wchar_t symbol = cell.get_symbol();
		UI::instance().enable_color_pair(color_pair);
		UI::instance().print_wide(y, x, symbol);
	}
	UI::instance().update();
	Log::log("Cave drawn");
}
