#include <vector>
#include <iomanip>
#include "Cell.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"

// returns cells with type rock and a density
std::vector<Cell> CaveGenerator::form_rock(const Cave& cave)
{
	std::vector<Cell> cells;
	for (size_t i = 0; i < SIZE; ++i)
	{
		size_t y = i / WIDTH;
		size_t x = i % WIDTH;
		double perlin = Random::noise3D(y, x, cave.get_level(), PERLIN_FREQUENCY, cave.get_seed());
		size_t distance_to_edge =
			std::min(
			std::min(x, y),
			std::min(WIDTH - x - 1, HEIGHT - y - 1));
		double edge_weigh = 1; // close to edges rock is denser
		if (distance_to_edge <= MARGIN)
			edge_weigh = Math::map(MARGIN - distance_to_edge, 0, MARGIN, 1, EDGE_WEIGH_MULT);
		const double mapped = Math::map(perlin, 0, 1, 1, ROCK_DENSITY * edge_weigh);
		cells.push_back(Cell(i, "rock", mapped));
	}
	return cells;
}

void CaveGenerator::print_cells(const std::vector<Cell>& cells)
{
	const char* colors[9] = {
    "\033[38;5;21m",  // 1 - dark blue
    "\033[38;5;27m",  // 2
    "\033[38;5;33m",  // 3
    "\033[38;5;39m",  // 4 - cyan-ish
    "\033[38;5;45m",  // 5
    "\033[38;5;51m",  // 6
    "\033[38;5;82m",  // 7 - green
    "\033[38;5;226m", // 8 - yellow
    "\033[38;5;196m"  // 9 - red
	};
	for (size_t i = 0; i < SIZE; ++i)
	{
		int dens = static_cast<int>(std::round(Math::map(cells[i].get_density(), 1, EDGE_WEIGH_MULT * ROCK_DENSITY, 1, 9)));
		if (dens < 1) dens = 1;
		if (dens > 9) dens = 9;
		std::cout << colors[dens] << std::setw(2) << dens;
		if (i % WIDTH == WIDTH - 1)
			std::cout << std::endl;
	}
}

Cave CaveGenerator::generate_cave(const size_t level)
{
	Cave cave;
	cave.set_level(level);
	cave.set_seed(Random::randint(10000,99999));

	std::vector<Cell> cells = form_rock(cave);
	print_cells(cells);
	return cave;
}
