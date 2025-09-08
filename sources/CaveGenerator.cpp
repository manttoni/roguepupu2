#include <vector>
#include <iomanip>
#include <ncurses.h>
#include "Cell.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"

CaveGenerator::CaveGenerator()
	: height(0), width(0), size(0), frequency(0), seed(0), octaves(0), margin(0) {}
CaveGenerator::CaveGenerator(const size_t height, const size_t width, const double frequency, const int seed, const int margin_percent, const int octaves)
	: height(height), width(width), size(height * width), frequency(frequency), seed(seed), octaves(octaves)
{
	margin = static_cast<size_t>(width * margin_percent / 100);
}
CaveGenerator::CaveGenerator(const CaveGenerator& other)
{
	height = other.height;
	width = other.width;
	size = other.size;
	frequency = other.frequency;
	seed = other.seed;
	caves = other.caves;
	octaves = other.octaves;
	margin = other.margin;
}

CaveGenerator CaveGenerator::operator=(const CaveGenerator& other)
{
	if (this != &other)
	{
		height = other.height;
		width = other.width;
		size = other.size;
		frequency = other.frequency;
		seed = other.seed;
		caves = other.caves;
		octaves = other.octaves;
		margin = other.margin;
	}
	return *this;
}

// returns cells with type rock and a density [1,9]
std::vector<Cell> CaveGenerator::form_rock(const size_t level)
{
	std::vector<Cell> cells;
	cells.reserve(size);
	for (size_t i = 0; i < size; ++i)
	{
		size_t y = i / width;
		size_t x = i % width;

		// [0,1]
		double perlin = Random::noise3D(y, x, level, frequency, seed, octaves);

		// distance to closest edge
		int distance_to_edge = std::min(std::min(x, y), std::min(width - x - 1, height - y - 1));

		// if close enough, make rock denser
		double edge_weight =
			distance_to_edge <= margin && margin != 0
			? Math::map(margin - distance_to_edge, 0, margin, 1, EDGE_WEIGH_MULT)
			: 1;


		// map perlin [0,1] * edge_weight [1,EDGE_WEIGH_MULT] to [1,9]
		double density = Math::map(perlin * edge_weight, 0, EDGE_WEIGH_MULT, 1, 9);
		cells.push_back(Cell(i, "rock", density));
	}
	return cells;
}

std::vector<Cell> CaveGenerator::find_water_path(Cave& cave)
{
	std::vector<Cell> cells = cave.get_cells();
	Cell start = cells[cave.get_water_start()];
	Cell end = cells[cave.get_water_end()];

	std::vector<Cell> open_set = {start};
	std::map<Cell, Cell> came_from;

	std::map<Cell, double> g_score;
	std::map<Cell, double> f_score;
	g_score[start] = 0;
	f_score[start] = cave.distance(start, end);

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
		for (Cell* neighbor : cave.get_neighbors(current))
		{
			double tentative_g_score = g_score[current] + neighbor->get_density();//cave.distance(current, *neighbor);
			if (g_score.count(*neighbor) == 0)
				g_score[*neighbor] = std::numeric_limits<double>::infinity();
			if (tentative_g_score < g_score[*neighbor])
			{
				came_from[*neighbor] = current;
				g_score[*neighbor] = tentative_g_score;
				f_score[*neighbor] = tentative_g_score + cave.distance(*neighbor, end);
				if (!Utils::contains(open_set, *neighbor))
					open_set.push_back(*neighbor);
			}
		}
	}
	return {};
}

// generate a cave and store it in vector
void CaveGenerator::generate_cave(const size_t level)
{
	// form a cave with just rock
	std::vector<Cell> cells = form_rock(level);
	Cave cave(height, width, cells, level, seed);

	// VERSION 1
	// get water start-end
	if (level == 1)
		cave.set_water_start(size / 2);
	else
		cave.set_water_start(caves[level - 2].get_water_end());

	cave.set_water_end(Random::randsize_t(0, size - 1));

	// carve tunnel
	std::vector<Cell> tunnel = find_water_path(cave);
	for (Cell& c : tunnel)
	{
		size_t id = c.get_id();
		cells[id] = Cell(id, "floor");
	}
	cave.set_cells(cells);
	caves.push_back(std::move(cave));
}

// generate all z levels until reaches target
Cave& CaveGenerator::get_cave(const size_t level)
{
	while (caves.size() < level)
		generate_cave(caves.size() + 1);

	return caves[level - 1];
}
