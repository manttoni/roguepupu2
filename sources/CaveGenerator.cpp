#include <vector>
#include <iomanip>
#include <ncurses.h>
#include "Cell.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"

CaveGenerator::CaveGenerator()
	: height(LINES - 1), width(COLS - 1), size(height * width)
{
	margin = static_cast<size_t>(width * MARGIN_PERCENT / 100);
	seed = Random::randint(10000, 99999);
}

// returns cells with type rock and a density
std::vector<Cell> CaveGenerator::form_rock(const size_t level)
{
	std::vector<Cell> cells;
	for (size_t i = 0; i < size; ++i)
	{
		size_t y = i / width;
		size_t x = i % width;
		double perlin = Random::noise3D(y, x, level, PERLIN_FREQUENCY, seed);
		size_t distance_to_edge = std::min(std::min(x, y), std::min(width - x - 1, height - y - 1));
		double edge_weigh = 1; // close to edges rock is denser
		if (distance_to_edge <= margin)
			edge_weigh = Math::map(margin - distance_to_edge, 0, margin, 1, EDGE_WEIGH_MULT);
		double density = Math::map(ROCK_DENSITY * edge_weigh, 1, EDGE_WEIGH_MULT * ROCK_DENSITY, 1, 9);
		const double mapped = Math::map(perlin, 0, 1, 1, density);
		cells.push_back(Cell(i, "rock", mapped));
	}
	return cells;
}

void CaveGenerator::generate_cave(const size_t level)
{
	std::vector<Cell> cells = form_rock(level);
	Cave cave(height, width, cells, level, seed);
	caves.push_back(cave);
}

Cave CaveGenerator::get_cave(const size_t level)
{
	if (level > caves.size())
		generate_cave(level);
	return caves[level - 1];
}
