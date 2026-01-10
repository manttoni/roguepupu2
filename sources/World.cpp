#include <exception>                     // for exception
#include <fstream>                       // for basic_ifstream, basic_istream
#include <nlohmann/detail/json_ref.hpp>  // for json_ref
#include <nlohmann/json.hpp>             // for basic_json, operator>>
#include <nlohmann/json_fwd.hpp>         // for json
#include <string>                        // for basic_string, operator+, string
#include "systems/EnvironmentSystem.hpp"
#include "systems/MovementSystem.hpp"
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
#include "systems/TransitionSystem.hpp"
#include "AbilityDatabase.hpp"
#include "Components.hpp"

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
	}
	catch (std::exception& e)
	{
		Log::error("Parsing world generation data went wrong");
	}

	registry.ctx().emplace<AbilityDatabase>();
}

// A* to find path of least resistance through solid rock
std::vector<size_t> World::find_water_path(const size_t start, const size_t end, const bool clamp_density)
{
	auto& canvas = caves.back();
	auto& cells = canvas.get_cells();

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
			const double density = clamp_density ?
				std::max(0.0, cells[neighbor_idx].get_density()) :
				cells[neighbor_idx].get_density();
			double tentative_g_score = g_score[current_idx] + density * density;

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
		double density = Math::map(perlin, 0, 1, 1, DENSITY_MAX);
		cells[i].set_type(Cell::Type::Rock);
		cells[i].set_density(density);
		cells[i].set_idx(i);
		cells[i].set_glyph(L' ');
	}
}

bool World::place_location(const Location& location)
{
	const double max_rd = static_cast<double>(width) / 2.0 - 10.0;
	double rd;
	if (location.radial_distance >= 0 && location.radial_distance <= 1)
		rd = location.radial_distance;
	else
	{
		const double min = 0.25; // keep center aka spawn area clear
		const double max = 1;
		rd = Random::randreal(min, max);
	}
	const double angle = Random::randreal(0.0, 2.0 * M_PI);
	const size_t cy = height / 2;
	const size_t cx = width / 2;
	const auto& vec2 = Math::polar_to_cartesian(Vec2(cy, cx), rd * max_rd, angle);
	const size_t place_idx = vec2.y * width + vec2.x;
	auto& canvas = caves.back();
	const auto& nearby_ids = canvas.get_nearby_ids(place_idx, location.radius);
	for (const auto idx : nearby_ids)
	{
		if (canvas.get_locations().contains(idx))
			return false;
	}
	canvas.add_location(place_idx, location);
	return true;
}

bool World::set_locations()
{
	LocationDatabase ld;
	const auto& set = ld.get_random_set();
	for (const auto& location : set)
		while (place_location(location) == false);

	// Check overlapping locations
	auto& canvas = caves.back();
	const auto& locations = canvas.get_locations();
	for (const auto& [idx, location] : locations)
	{
		const auto nearby_ids = canvas.get_nearby_ids(idx, location.radius); // does not include itself
		for (const auto nearby : nearby_ids)
		{
			if (locations.contains(nearby))
				return false;
		}
	}

	return true;
}

void World::populate_locations()
{
	auto& canvas = caves.back();
	const auto& locations = canvas.get_locations();
	for (const auto& [idx, location] : locations)
	{
		// Cells within radius
		auto cell_ids = canvas.get_nearby_ids(idx, location.radius);
		std::shuffle(cell_ids.begin(), cell_ids.end(), Random::rng());

		// Entity structs with id and amount/spawn info
		const auto& entities = location.entities;

		// index of nearby cells vector
		size_t i = 0;

		// loop spawn info
		for (const auto& entity : entities)
		{
			// loop for amount
			for (size_t j = 0; j < entity.amount; ++j)
			{
				// get next empty cell
				while (i < cell_ids.size() && !canvas.get_cell(cell_ids[i]).is_empty()) i++;
				if (i >= cell_ids.size())
					return;
				size_t spawn_idx = cell_ids[i];
				if (entity.amount == 1 && entity.radial_distance >= 0 && entity.radial_distance <= location.radius)
				{
					const auto coord = Math::polar_to_cartesian(
							Vec2(idx / width, idx % width),
							location.radius * entity.radial_distance,
							Random::randreal(0, 2 * M_PI));
					spawn_idx = coord.y * width + coord.x;
				}
				const auto e = EntityFactory::instance().create_entity(registry, entity.id, &canvas.get_cell(spawn_idx));
				registry.emplace<BaseLocation>(e, idx, location.radius);
				if (registry.get<Category>(e).category == "creatures")
					canvas.get_npcs().push_back(e);
			}
		}
	}
}

bool World::all_connected()
{
	auto& canvas = caves.back();
	const auto& locations = canvas.get_locations();
	const size_t source_idx = height / 2 * width + width / 2;
	for (const auto& [idx, location] : locations)
	{
		const auto& path = MovementSystem::find_path(&canvas, source_idx, idx, false);
		if (path.empty())
			return false;
	}
	return true;
}

void World::form_tunnels()
{
	auto& canvas = caves.back();
	const auto& locations = canvas.get_locations();
	const size_t source_idx = height / 2 * width + width / 2;
	while (!all_connected())
	{
		for (const auto& [idx, location] : locations)
		{
			find_water_path(source_idx, idx);
		}
	}
}

void World::set_rock_colors()
{
	auto& canvas = caves.back();
	for (auto& cell : canvas.get_cells())
	{
		if (cell.get_type() != Cell::Type::Rock)
			continue;
		const short channel = 100 * static_cast<short>(std::ceil(cell.get_density()));
		cell.set_bgcolor(Color(channel, channel, channel));
	}
}

void World::set_humidity()
{
	auto& canvas = caves.back();
	const size_t z = canvas.get_level();
	for (const auto cell_idx : get_empty_cells(canvas))
	{
		const size_t y = cell_idx / width;
		const size_t x = cell_idx % width;
		const double value = Random::noise3D(y, x, z, 0.25, seed, 1);
		canvas.get_cell(cell_idx).set_humidity(value);
	}
}

// Get cells that have nothing else than a floor
std::vector<size_t> World::get_empty_cells(const Cave& cave)
{
	std::vector<size_t> empty_cells;
	for (const auto& cell : cave.get_cells())
	{
		if (cell.is_empty())
			empty_cells.push_back(cell.get_idx());
	}
	return empty_cells;
}

void World::spawn_entities(nlohmann::json& filter)
{
	auto& canvas = caves.back();
	const auto z = canvas.get_level();
	filter["spawn"] = "any";
	filter["player"] = "none";
	const auto& names = EntityFactory::instance().random_pool(filter, SIZE_MAX);
	const auto& LUT = EntityFactory::instance().get_LUT();
	auto empty_cells = get_empty_cells(canvas);
	std::shuffle(empty_cells.begin(), empty_cells.end(), Random::rng());
	auto& cells = canvas.get_cells();
	for (const auto& cell_idx : empty_cells)
	{
		Cell& cell = cells[cell_idx];
		const size_t y = cell_idx / width;
		const size_t x = cell_idx % width;
		for (const auto& name : names)
		{
			if (!LUT.contains(name))
				Log::error("EntityFactory LUT error with " + name);
			const auto& data = LUT.at(name);
			const auto& spawn = data["spawn"];
			if (spawn.contains("chance"))
			{
				const double frequency = spawn.contains("frequency") ? spawn["frequency"].get<double>() : Random::randreal(0, 1);
				const double octaves = spawn.contains("octaves") ? spawn["octaves"].get<size_t>() : Random::randsize_t(1, 8);
				const double random = Random::noise3D(y, x, z, frequency, seed, octaves);
				if (random > spawn["chance"].get<double>())
					continue;
			}
			if (spawn.contains("near"))
			{
				const auto& near = spawn["near"].get<std::string>();
				if (near == "rock")
				{
					if (!canvas.neighbor_has_type(cell_idx, Cell::Type::Rock))
						continue;
				}
				else Log::error("Unknown near: " + near);
			}
			if (spawn.contains("space"))
			{
				const auto& space = spawn["space"].get<std::string>();
				double radius;
				if (space == "wide")
					radius = 5;
				else if (space == "narrow")
					radius = 1;
				else Log::error("Unknown space: " + space);

				const auto& nearby = canvas.get_nearby_ids(cell_idx, radius);
				double empty_space = 0;
				for (const auto idx : nearby)
				{
					if (cells[idx].is_empty())
						empty_space++;
				}
				const double total_space = nearby.size();

				if (space == "wide" && empty_space / total_space < 0.95)
					continue;
				if (space == "narrow" && total_space - empty_space < 2)
					continue;
			}
			if (spawn.contains("light"))
			{
				const auto& light = spawn["light"];
				const auto& cell_lights = cell.get_lights();
				size_t light_amount = 0;
				for (const auto& [color, stacks] : cell_lights)
					if (stacks > 0)
						light_amount += stacks * (color.get_r() + color.get_g() + color.get_b());

				if (light == "dark" && light_amount > 0.0)
					continue;
			}
			if (spawn.contains("humidity"))
			{
				const auto& spawn_humidity = spawn["humidity"].get<double>();
				if (spawn_humidity > cell.get_humidity())
					continue;
			}

			// This entity really deserves to spawn here
			entt::entity e = EntityFactory::instance().create_entity(registry, name, &cell);
			caves.back().reset_lights();
			if (registry.get<Category>(e).category == "creatures")
				canvas.get_npcs().push_back(e);
			break;
		}
	}
}

void World::normalize_negative_density()
{
	// if density is <= 0, its type is floor
	// If player mines rock later, its density will become 0.
	// Negative density represents concave floor, while positive is rock wall
	// Water erosion simulation is the cause of negative density
	double deepest = 0.0; // 0.0 is biggest possible value for empty cells
	Cave& canvas = caves.back();
	auto& cells = canvas.get_cells();
	for (const auto cell_idx : get_empty_cells(canvas))
	{
		const double density = cells[cell_idx].get_density();
		assert(density <= 0.0);
		deepest = std::min(deepest, density);
	}
	for (const auto cell_idx : get_empty_cells(canvas))
	{
		const double density = cells[cell_idx].get_density();
		cells[cell_idx].set_density(Math::map(density, deepest, 0.0, -0.9, 0.0));
	}
}

void World::smooth_elevation()
{
	Cave& canvas = caves.back();
	auto& cells = canvas.get_cells();
	auto empty_cells = canvas.get_empty_cells();
	std::shuffle(empty_cells.begin(), empty_cells.end(), Random::rng());
	for (const auto cell_idx : empty_cells)
	{
		// make cell less deep, then deepen cells around it
		auto& cell = cells[cell_idx];
		assert(cell.get_density() <= 0);
		double reduced = cell.get_density() / 16;
		cell.reduce_density(reduced);
		assert(cell.get_density() <= 0);

		const auto around = canvas.get_nearby_ids(cell_idx, 1.5);
		for (const auto around_idx : around)
		{
			if (cells[around_idx].is_empty())
				cells[around_idx].reduce_density(reduced / around.size() * -1);
		}
	}
}

void World::add_water()
{
	Cave& canvas = caves.back();
	auto& cells = canvas.get_cells();
	for (const auto cell_idx : get_empty_cells(canvas))
	{
		auto& cell = cells[cell_idx];
		const double amount = cell.get_humidity() * cell.get_density() * -1;
		if (amount < canvas.get_humidity())
			continue;
		auto& mix = cell.get_liquid_mixture();
		mix.add_liquid(Liquid::Type::Water, amount);
	}
}

void World::set_sink()
{
	auto& canvas = caves.back();
	auto& cells = canvas.get_cells();
	const auto sink = ECS::get_sink(registry, canvas);
	assert(sink != entt::null);
	Cell* sink_cell = ECS::get_cell(registry, sink);
	assert(sink_cell != nullptr);
	size_t sink_idx = sink_cell->get_idx();

	// Find deepest spot in the same hole
	bool flag = true;
	while (flag)
	{
		flag = false;
		const auto nearby = canvas.get_nearby_ids(sink_idx, 1.5);
		double density = cells[sink_idx].get_density();
		for (const auto n : nearby)
		{
			if (!MovementSystem::can_move(canvas, sink_idx, n))
				continue;
			const double n_density = cells[n].get_density();
			if (n_density < density)
			{
				density = n_density;
				sink_idx = n;
				flag = true;
			}
		}
	}

	// Clear the whole cell
	for (const auto cell_entity : cells[sink_idx].get_entities())
		ECS::destroy_entity(registry, cell_entity);

	// Set it as position of sink
	registry.replace<Position>(sink, &cells[sink_idx]);
	sink_cell = &cells[sink_idx];
	sink_cell->set_bgcolor(Color{});
	sink_cell->set_density(-1000);
}

void World::generate_cave(const size_t level)
{
	UI::instance().dialog("Generating cave...");
	Log::log("Generating cave " + std::to_string(level));

	rng.seed(seed + level);
	caves.emplace_back(level, height, width, seed);
	Cave& canvas = caves.back();
	canvas.set_world(this);

	UI::instance().dialog("Generating terrain...");
	Log::log("Generating terrain...");

	// Form solid rock everywhere
	Log::log("Forming rock...");
	form_rock();

	// Set locations randomized from locations.json
	Log::log("Setting locations...");
	while (set_locations() == false)
		caves.back().clear_locations();
	assert(canvas.get_locations().size() > 0);

	// Connect all locations by water erosion algorithm
	Log::log("Forming tunnels...");
	form_tunnels();

	// Give rock colors based on density
	Log::log("Setting rock colors...");
	set_rock_colors();

	// Give each cell a humidity value, which affects spawning chance and water level
	Log::log("Setting humidity...");
	set_humidity();

	// Forced values
	Log::log("Forcing source/sink values...");
	canvas.get_cell(canvas.get_source_idx()).set_density(0);

	// Make elevation smooth to make nicer puddles
	Log::log("Smoothing elevation...");
	for (size_t i = 0; i < 64; ++i) smooth_elevation();

	// Elevation should be [-0.9, 0.0]
	Log::log("Normalizing elevation...");
	normalize_negative_density();

	// Spawn water
	Log::log("Spawning water...");
	add_water();

	Log::log("Terrain generated");

	// Spawn entities
	UI::instance().dialog("Spawning entities...");
	Log::log("Spawning entities...");
	populate_locations();
	std::vector<nlohmann::json> filters =
	{
		{{"subcategory", "mushrooms"}},
		{{"category", "creatures"}}
	};
	for (auto& filter : filters)
		spawn_entities(filter);

	Log::log("Entities spawned");

	set_sink();

	UI::instance().dialog("Simulating environment...");
	for (size_t i = 0; i < 10000; ++i)
		EnvironmentSystem::simulate_environment(&canvas);

	Log::log("Cave " + std::to_string(level) + " generated");
	return;
}

Cave& World::get_cave(const size_t level)
{
	while (caves.size() < level)
		generate_cave(caves.size() + 1);

	return caves[level - 1];
}
