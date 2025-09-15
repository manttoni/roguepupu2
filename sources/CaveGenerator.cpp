#include <vector>
#include <iomanip>
#include <ncurses.h>
#include <panel.h>
#include "Cell.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"
#include "UI.hpp"
#include "Fungus.hpp"

CaveGenerator::CaveGenerator()
	: height(0), width(0), size(0), frequency(0), seed(0), octaves(0), margin(0) {}

CaveGenerator::CaveGenerator(
		const size_t height,
		const size_t width,
		const double frequency,
		const int seed,
		const int margin_percent,
		const int octaves,
		const double A, const double B, const double C,
		const size_t fungus_spawn_chance)
	: height(height), width(width), size(height * width), frequency(frequency), seed(seed), octaves(octaves), rng(seed), erosion_a(A), erosion_b(B), erosion_c(C), fungus_spawn_chance(fungus_spawn_chance)
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
	erosion_a = other.erosion_a;
	erosion_b = other.erosion_b;
	erosion_c = other.erosion_c;
	fungus_spawn_chance = other.fungus_spawn_chance;
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
		erosion_a = other.erosion_a;
		erosion_b = other.erosion_b;
		erosion_c = other.erosion_c;
		fungus_spawn_chance = other.fungus_spawn_chance;
	}
	return *this;
}


// A* to find path of least resistance through solid rock
// optimize later to use the indices instead of cells
std::vector<size_t> CaveGenerator::find_water_path()
{
	assert(erosion_a != 0);
	assert(erosion_b != 0);
	assert(erosion_c != 0);
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

		// A erosion
		// Erosion here is very large scale and interesting
		canvas.get_cells()[current_idx].reduce_density(erosion_a);

		Utils::remove_element(open_set, current_idx);
		for (const size_t neighbor_idx : canvas.get_nearby_ids(current_idx, 1.5))
		{
			double tentative_g_score = g_score[current_idx] + cells[neighbor_idx].get_density();

			// if neighbor doesnt have a g_score, init it to inf
			if (g_score.count(neighbor_idx) == 0)
				g_score[neighbor_idx] = std::numeric_limits<double>::infinity();

			// B erosion
			// erosion here will make smooth cave like C
			cells[neighbor_idx].reduce_density(erosion_b);

			if (tentative_g_score < g_score[neighbor_idx])
			{
				// C erosion
				// same as B
				cells[neighbor_idx].reduce_density(erosion_c);

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
	top_panel(CaveView::cave_panel);
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
		cells[i].set_idx(i);
	}
}

void CaveGenerator::set_source_sink()
{
	if (canvas.get_level() == 1)
		canvas.set_source(height / 2 * width + width / 2);
	else
		canvas.set_source(caves.back().get_sink()); // sink of last level

	size_t sink_idx;
	do
		sink_idx = Random::randsize_t(0, size - 1, rng);
	while (canvas.distance(canvas.get_source(), sink_idx) < width / 2);
	canvas.set_sink(sink_idx);
}

// Glowing fungi grow next to walls
// Woody fungi grow if there is enough space
void CaveGenerator::spawn_fungi()
{
	const double WOODY_RADIUS = 5;
	const double WOODY_SPACE_RATIO = 0.75;
	auto& cells = canvas.get_cells();
	for (size_t i = 0; i < size; ++i)
	{
		if (Random::randsize_t(0, 100, rng) > fungus_spawn_chance)
			continue;

		Cell& cell = cells[i];
		if (cell.get_type() == Cell::Type::FLOOR)
		{
			if (canvas.neighbor_has_type(i, Cell::Type::ROCK))
			{
				cell.add_entity(Fungus(Fungus::Type::GLOWING));
				continue;
			}
			const auto& nearby = canvas.get_nearby_ids(i, WOODY_RADIUS);
			double space = 0;
			for (const auto& idx : nearby)
				if (!cells[idx].blocks_movement())
					space++;
			double a = 3.14 * WOODY_RADIUS * WOODY_RADIUS;
			if (space / a > WOODY_SPACE_RATIO)
				cell.add_entity(Fungus(Fungus::Type::WOODY));
		}

	}
}

void CaveGenerator::color_cells()
{
	short black_id = UI::instance().add_color(0, 0, 0);
	std::map<size_t, short> density_color_pair_ids;
	for (size_t i = 1; i <= 9; ++i)
	{
		short val = i * 100;
		short color_id = UI::instance().add_color(val, val, val);
		density_color_pair_ids[i] = UI::instance().add_color_pair(color_id, black_id);
	}

	auto& cells = canvas.get_cells();
	for (Cell& cell : cells)
	{
		switch (cell.get_type())
		{
			case Cell::Type::NONE:
				break;
			case Cell::Type::ROCK:
			{
				const size_t density = static_cast<size_t>(std::ceil(cell.get_density()));
				cell.set_color_pair_id(density_color_pair_ids[density]);
				break;
			}
			case Cell::Type::FLOOR:
				cell.set_color_pair_id(UI::instance().DEFAULT);
				break;
			case Cell::Type::SOURCE:
				cell.set_color_pair_id(UI::instance().DEFAULT);
				break;
			case Cell::Type::SINK:
				cell.set_color_pair_id(UI::instance().DEFAULT);
				break;
		}
	}
}

void CaveGenerator::generate_cave(const size_t level)
{
	rng.seed(seed + level);
	canvas = Cave(level, height, width, seed);
	form_rock();
	set_source_sink();
	form_tunnels();
	spawn_fungi();
	color_cells();

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
