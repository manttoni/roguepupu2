#include <string>
#include <cmath>
#include <cassert>
#include <iostream>
#include "Area.hpp"
#include "Cell.hpp"
#include "Utils.hpp"

std::pair<size_t, size_t> Area::get_stream() const
{
	const int margin = 5;
	const double min_distance = distance(margin * height + margin, (height - margin) * width + (width - margin)) / 2;

	std::pair<size_t, size_t> stream;
	stream.first = Random::randint(margin, height - margin) * width + Random::randint(margin, width - margin);
	stream.second = stream.first;
	while (distance(stream.first, stream.second) < min_distance)
		stream.second = Random::randint(margin, height - margin) * width + Random::randint(margin, width - margin);
	return stream;
}


/* CONSTRUCTORS */
Area::Area(const size_t height, const size_t width) : height(height), width(width)
{

	// world of solid rock with thickness from Perlin noise
	const double frequency = 0.1;
	const int octaves = 8;
	const int max_rock_thickness = 9; // affects also cave width, so can be used as margin
//	const double space = Random::randreal(0.25, 0.5); // at start its 0, fully open is 1
	const size_t source_count = 1;//Random::randint(1, 2);
	const size_t sink_count = 1;//Random::randint(2, 4);

	for (size_t i = 0; i < get_size(); ++i)
	{
		double perlin = Random::noise(i / width, i % width, frequency, octaves);
		int mapped = Math::map(perlin, 0, 1, 1, max_rock_thickness);
		cells.push_back(Cell(i, "rock", mapped));
	}

	// water erodes rock to create passages
	std::vector<size_t> sources, sinks;
	for (size_t i = 0; i < source_count; ++i)
		sources.push_back(Random::randint(max_rock_thickness, height - max_rock_thickness) * width + Random::randint(max_rock_thickness, width - max_rock_thickness));
	for (size_t i = 0; i < sink_count; ++i)
		sinks.push_back(Random::randint(max_rock_thickness, height - max_rock_thickness) * width + Random::randint(max_rock_thickness, width - max_rock_thickness));
	std::vector<std::vector<Cell>> stream_paths;
	for (const size_t source : sources)
	{
		for (const size_t sink : sinks)
		{
			stream_paths.push_back(find_path(cells[source], cells[sink], true));
		}
	}
//	while((double)count_cells("floor") / count_cells("rock") < space && streams.size() < max_streams)
	//for (size_t i = 0; i < stream_count; ++i)
	for (auto& path : stream_paths)
	{
		for (Cell& c : path)
		{
			if (c.get_type() != "rock")
				continue;
			int rock_thickness = c.get_rock_thickness();
			auto nearby_cells = get_nearby_cells(c, rock_thickness); // if eroded rock is very thick, it affects a larger area
			for (Cell* nearby : nearby_cells)
				nearby->erode(rock_thickness);
		}
	}

	// mark stream start and end on map
	for (const size_t source : sources)
		cells[source] = Cell(source, "stream_source");
	for (const size_t sink : sinks)
		cells[sink] = Cell(sink, "stream_sink");
	print_area(stream_paths);
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
				type = "rock";
				break;
		}
		cells.push_back(Cell(i, type));
	}
}

/* CELL TO CELL */
std::vector<Cell> Area::find_path(const Cell &start, const Cell &end, const bool water)
{
	if (start.is_blocked() && !water)
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
			{	// assign the cell from where we got to to current
				current = came_from[current];
				path.push_back(current);
			}
			return path;
		}

		Utils::remove_element(open_set, current);
		for (Cell* neighbor : get_neighbors(current))
		{
			if (!water && !has_access(current, *neighbor))
				continue;
			double tentative_g_score = g_score[current] + (water ? neighbor->get_rock_thickness() : distance(current, *neighbor));
			if (water) neighbor->erode(1);
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

void Area::print_area(std::vector<std::vector<Cell>> paths) // for testing
{
	std::cout << std::string(width * 2 + 1, '=') << std::endl;
	for (size_t i = 0; i < height * width; ++i)
	{
		if (i % width == 0)
			std::cout << "|";
		if (cells[i].get_type() == "floor")
			std::cout << Colors::BLACK;

		// check if is on a stream path
		for (const auto& path : paths)
			for (const auto& cell : path)
				if (cell.get_id() == i)
					std::cout << Colors::BLUE;


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
	return distance(start_id, end_id);
}

double Area::distance(const size_t start_id, const size_t end_id) const
{
	int start_y = start_id / width;
	int start_x = start_id % width;
	int end_y = end_id / width;
	int end_x = end_id % width;

	return std::hypot(start_y - end_y, start_x - end_x);
}

std::vector<Cell*> Area::get_nearby_cells(const Cell &middle, const int r)
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

std::vector<Cell*> Area::get_neighbors(const Cell &middle)
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

size_t Area::count_cells(const std::string& type) const
{
	size_t count = 0;
	for (size_t i = 0; i < get_size(); ++i)
	{
		if (cells[i].get_type() == type)
			count++;
	}
	return count;
}
