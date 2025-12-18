#include <exception>                     // for exception
#include <fstream>                       // for basic_ifstream, basic_istream
#include <nlohmann/detail/json_ref.hpp>  // for json_ref
#include <nlohmann/json.hpp>             // for basic_json, operator>>
#include <nlohmann/json_fwd.hpp>         // for json
#include <string>                        // for basic_string, operator+, string
#include "Cave.hpp"                      // for Cave
#include "Cell.hpp"                      // for Cell
#include "Color.hpp"                     // for Color
#include "ECS.hpp"                       // for get_level
#include "EntityFactory.hpp"             // for EntityFactory
#include "UI.hpp"                        // for UI
#include "Utils.hpp"                     // for randsize_t, error, noise3D
#include "World.hpp"                     // for World
#include "entt.hpp"                      // for vector, size_t, deque, map
#include "systems/InventorySystem.hpp"   // for add_item

World::World() : seed(Random::randsize_t(10000, 99999)), rng(seed)
{
	const std::string path = "data/world/generation.json";
	std::ifstream file(path);
	if (!file.is_open())
		Log::error(std::string("Opening file failed: ") + path);
	nlohmann::json data;
	file >> data;
	file.close();
	try
	{
		height = data["height"].get<int>();
		width = data["width"].get<int>();

		const auto& noise = data["noise"];
		frequency = noise["frequency"].get<double>();
		octaves = noise["octaves"].get<int>();
		erosion_a = noise["erosion_a"].get<double>();
		erosion_b = noise["erosion_b"].get<double>();
		erosion_c = noise["erosion_c"].get<double>();

		const auto& mushrooms = data["mushrooms"];
		mushroom_spawn_chance = mushrooms["spawn_chance"].get<double>();
		mushroom_frequency = mushrooms["frequency"].get<double>();
		mushroom_octaves = mushrooms["octaves"].get<size_t>();
		mushroom_woody_radius = mushrooms["woody_radius"].get<size_t>();
		mushroom_woody_space_ratio = mushrooms["woody_space_ratio"].get<double>();

		const auto& chests = data["chests"];
		chest_spawn_chance = chests["spawn_chance"].get<double>();
		chest_value_power = chests["value_power"].get<size_t>();
		chest_value_multiplier = chests["value_multiplier"].get<size_t>();
		chest_value_scalar = chests["value_scalar"].get<int>();
		chest_item_variance = chests["item_variance"].get<size_t>();
	}
	catch (std::exception& e)
	{
		Log::error("Parsing world generation data went wrong");
	}
}

// A* to find path of least resistance through solid rock
std::vector<size_t> World::find_water_path()
{
	auto& canvas = caves.back();
	auto& cells = canvas.get_cells();
	size_t start = canvas.get_source_idx();
	size_t end = canvas.get_sink_idx();

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
			double tentative_g_score = g_score[current_idx] + cells[neighbor_idx].get_density() * cells[neighbor_idx].get_density();

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

void World::form_tunnels()
{
	auto& canvas = caves.back();
	while (canvas.find_path(canvas.get_source_idx(), canvas.get_sink_idx()).empty())
	{
		find_water_path();
	}
}

void World::form_rock()
{
	auto& canvas = caves.back();
	size_t level = canvas.get_level();
	auto& cells = canvas.get_cells();
	for (size_t i = 0; i < height * width; ++i)
	{
		size_t y = i / width;
		size_t x = i % width;
		double perlin = Random::noise3D(y, x, level, frequency, seed, octaves);
		double density = Math::map(perlin, 0, 1, 1, 9);
		cells[i].set_type(Cell::Type::ROCK);
		cells[i].set_density(density);
		cells[i].set_idx(i);
		cells[i].set_glyph(L'█');
	}
}

void World::set_source_sink()
{
	auto& canvas = caves.back();
	if (canvas.get_level() == 1)
		canvas.set_source_idx(height / 2 * width + width / 2);
	else
		canvas.set_source_idx((caves.end() - 2)->get_sink_idx()); // sink of last level

	size_t sink_idx;
	do
		sink_idx = Random::randsize_t(0, height * width - 1, rng);
	while
		(canvas.distance(canvas.get_source_idx(), sink_idx) < width / 2);
	canvas.set_sink_idx(sink_idx);
}

// Glowing fungi grow next to walls
// Woody fungi grow if there is enough space
void World::spawn_mushrooms()
{
	auto& canvas = caves.back();
	auto& cells = canvas.get_cells();

	for (size_t i = 0; i < height * width; ++i)
	{
		if (cells[i].blocks_movement())
			continue;
		if (Random::noise3D(i / width, i % width, canvas.get_level(), mushroom_frequency, seed, mushroom_octaves) > mushroom_spawn_chance
			|| Random::randsize_t(0, 100, rng) >= mushroom_spawn_chance * 100)
			continue;

		Cell& cell = cells[i];
		if (cell.get_type() != Cell::Type::FLOOR)
			continue;

		if (canvas.neighbor_has_type(i, Cell::Type::ROCK))
		{
			EntityFactory::instance().create_entity(registry, "glowing mushroom", &cell);
			continue;
		}
		const auto& nearby = canvas.get_nearby_ids(i, mushroom_woody_radius);
		double space = 0;
		for (const auto& idx : nearby)
			if (!cells[idx].blocks_movement())
				space++;
		double a = 3.14 * mushroom_woody_radius * mushroom_woody_radius;
		if (space / a > mushroom_woody_space_ratio)
			EntityFactory::instance().create_entity(registry, "woody mushroom", &cell);

	}
}

void World::spawn_chests()
{
	auto& canvas = caves.back();
	auto& cells = canvas.get_cells();

	for (size_t i = 0; i < height * width; ++i)
	{
		if (cells[i].blocks_movement() || Random::randreal(0, 1) > chest_spawn_chance)
			continue;

		nlohmann::json loot_filter = {{"category", "items"}};
		const auto& loot_pool = EntityFactory::instance().random_pool(loot_filter, chest_item_variance);
		const auto chest = EntityFactory::instance().create_entity(registry, "chest", &cells[i]);
		size_t amount = Random::randsize_t(0, 5);
		for (size_t i = 0; i < amount; ++i)
		{
			const size_t rand_idx = Random::randsize_t(0, loot_pool.size() - 1);
			assert(rand_idx < loot_pool.size());
			const auto item = EntityFactory::instance().create_entity(registry, loot_pool[rand_idx]);
			InventorySystem::add_item(registry, chest, item);
		}
	}
}

void World::spawn_creatures()
{
	auto& canvas = caves.back();
	auto& cells = canvas.get_cells();
	const size_t enemy_levels = 50 * canvas.get_level();
	std::vector<size_t> spawn_cells;

	for (size_t i = 0; i < height * width; ++i)
	{
		if (!cells[i].blocks_movement())
			spawn_cells.push_back(i);
	}

	std::shuffle(spawn_cells.begin(), spawn_cells.end(), rng);
	const nlohmann::json& filter =
	{
		{"category", "creatures"}
	};
	const auto& creature_pool = EntityFactory::instance().random_pool(filter, 3);
	size_t level_sum = 0;
	size_t spawn_idx = 0;
	while (level_sum < enemy_levels && spawn_idx < spawn_cells.size())
	{
		const size_t idx = spawn_cells[spawn_idx];
		const size_t creature_idx = Random::randsize_t(0, creature_pool.size() - 1);
		Cell* cell = &cells[idx];
		assert(cell != nullptr);
		assert(creature_idx < creature_pool.size());
		const entt::entity e = EntityFactory::instance().create_entity(registry, creature_pool[creature_idx], cell);
		level_sum += ECS::get_level(registry, e);
		spawn_idx++;
	}
}

void World::set_rock_colors()
{
	auto& canvas = caves.back();
	for (auto& cell : canvas.get_cells())
	{
		if (cell.get_type() != Cell::Type::ROCK)
			continue;
		const short channel = 100 * static_cast<short>(std::ceil(cell.get_density()));
		cell.set_fg(Color(channel, channel, channel));
	}
}

void World::generate_cave(const size_t level)
{
	UI::instance().dialog("Generating cave...");

	rng.seed(seed + level);
	caves.emplace_back(level, height, width, seed);
	caves.back().set_world(this);

	form_rock();
	set_source_sink();
	form_tunnels();
	spawn_mushrooms();
	spawn_chests();
	spawn_creatures();
	set_rock_colors();

	caves.back().apply_lights();
	return;
}

// generate all z levels until reaches target
Cave& World::get_cave(const size_t level)
{
	while (caves.size() < level)
		generate_cave(caves.size() + 1);

	return caves[level - 1];
}
