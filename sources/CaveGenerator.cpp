#include <vector>
#include <iomanip>
#include <ncurses.h>
#include <panel.h>
#include "Cell.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"
#include "UI.hpp"

CaveGenerator::CaveGenerator()
	: height(0), width(0), size(0), frequency(0), seed(0), octaves(0), margin(0) {}
CaveGenerator::CaveGenerator(const size_t height, const size_t width, const double frequency, const int seed, const int margin_percent, const int octaves)
	: height(height), width(width), size(height * width), frequency(frequency), seed(seed), octaves(octaves)
{
	margin = static_cast<size_t>(height * margin_percent / 100);
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


// A* to find path of least resistance through solid rock
// optimize later to use the indices instead of cells
std::vector<size_t> CaveGenerator::find_water_path()
{
	auto& cells = canvas.get_cells();
	size_t start = canvas.get_source();
	size_t end = canvas.get_sink();

	std::vector<size_t> open_set = { start };
	std::map<size_t, size_t> came_from;

	std::map<size_t, double> g_score;
	std::map<size_t, double> f_score;
	g_score[start] = 0;
	f_score[start] = canvas.distance(start, end);
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

		// C erosion
		// Erosion here is very large scale and interesting
		canvas.get_cells()[current_idx].reduce_density(.1);

		Utils::remove_element(open_set, current_idx);
		for (const size_t neighbor_idx : canvas.get_nearby_ids(current_idx, 1.5))
		{
			double tentative_g_score = g_score[current_idx] + cells[neighbor_idx].get_density();

			// if neighbor doesnt have a g_score, init it to inf
			if (g_score.count(neighbor_idx) == 0)
				g_score[neighbor_idx] = std::numeric_limits<double>::infinity();

			// A erosion
			// erosion here will produce same result as B
			cells[neighbor_idx].reduce_density(.01);

			if (tentative_g_score < g_score[neighbor_idx])
			{
				// B erosion
				// erosion here produces ok tunnel
				// same as A
				cells[neighbor_idx].reduce_density(.001);

				came_from[neighbor_idx] = current_idx;
				g_score[neighbor_idx] = tentative_g_score;
				f_score[neighbor_idx] = tentative_g_score + canvas.distance(neighbor_idx, end);
				if (!Utils::contains(open_set, neighbor_idx))
					open_set.push_back(neighbor_idx);
			}
		}
	}
	return {};
}

void CaveGenerator::form_tunnels()
{
	while (canvas.find_path(canvas.get_source(), canvas.get_sink()).empty())
	{
		find_water_path();
	}
}

void CaveGenerator::form_rock()
{
	size_t level = canvas.get_level();
	auto& cells = canvas.get_cells();
	for (size_t i = 0; i < size; ++i)
	{
		size_t y = i / width;
		size_t x = i % width;
		double perlin = Random::noise3D(y, x, level, frequency, seed, octaves);
		int distance_to_edge = std::min(std::min(x, y), std::min(width - x - 1, height - y - 1));

		// if close enough, make rock denser
		double edge_weight =
			distance_to_edge <= margin && margin != 0
			? Math::map(margin - distance_to_edge, 0, margin, 1, EDGE_WEIGH_MULT)
			: 1;

		double density = Math::map(perlin * edge_weight, 0, EDGE_WEIGH_MULT, 1, 9);
		cells[i].set_type(Cell::Type::ROCK);
		cells[i].set_density(density);
		cells[i].set_blocked(true);
	}
}

void CaveGenerator::generate_cave(const size_t level)
{
	// reset canvas. It will also select points for water source and sink
	canvas = Cave(level, height, width, seed);

	// Set water source and sink
	if (level == 1)	canvas.set_source(height / 2 * width + width / 2);
	else			canvas.set_source(caves.back().get_sink()); // sink of last level
	canvas.set_sink(Random::randsize_t(0, size)); // for now, sink is a random point

	// "paint" rock with different densities
	form_rock();

	// "paint" tunnels using water erosion simulation
	form_tunnels();

	// don't copy, just move
	caves.push_back(std::move(canvas));
	return;
}

// generate all z levels until reaches target
Cave& CaveGenerator::get_cave(const size_t level)
{
	while (caves.size() < level)
		generate_cave(caves.size() + 1);

	return caves[level - 1];
}
