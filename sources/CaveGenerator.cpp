#include <vector>
#include <iomanip>
#include <ncurses.h>
#include "Cell.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"

CaveGenerator::CaveGenerator()
	: height(0), width(0), size(0), smoothness(0), seed(0), octaves(0), margin(0) {}
CaveGenerator::CaveGenerator(const size_t height, const size_t width, const double smoothness, const size_t seed, const size_t margin_percent, const int octaves)
	: height(height), width(width), size(height * width), smoothness(smoothness), seed(seed), octaves(octaves)
{
	margin = static_cast<size_t>(width * margin_percent / 100);
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
		double perlin = Random::noise3D(y, x, level, smoothness, seed, octaves);

		// distance to closest edge
		size_t distance_to_edge = std::min(std::min(x, y), std::min(width - x - 1, height - y - 1));

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

// generate a cave and store it in vector
void CaveGenerator::generate_cave(const size_t level)
{
	std::vector<Cell> cells = form_rock(level);
	Cave cave(height, width, cells, level, seed);
	caves.push_back(cave);
}

// generate all z levels until reaches target
Cave CaveGenerator::get_cave(const size_t level)
{
	while (caves.size() < level)
		generate_cave(caves.size() + 1);

	return caves[level - 1];
}
