#include <algorithm>
#include <vector>
#include <map>
#include <cassert>
#include "utils/Parser.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "components/Components.hpp"
#include "domain/Cave.hpp"
#include "nlohmann/json.hpp"
#include "database/EntityFactory.hpp"
#include "utils/Random.hpp"
#include "utils/ECS.hpp"
#include "UI/UI.hpp"

namespace EntitySpawner
{
	void spawn_entities(entt::registry& registry, nlohmann::json filter, const size_t cave_idx)
	{
		// Add to filter that spawn data is needed
		filter["spawn"] = "any";

		const auto& cave = ECS::get_cave(registry, cave_idx);

		// Get all entities names that match the filter
		auto id_pool = EntityFactory::instance().random_pool(filter);
		assert(id_pool.size() > 0);

		// Get look up table which contains all info from json
		const auto& LUT = EntityFactory::instance().get_LUT();

		// By decision allow spawning on floor cells only
		auto floor_cells = cave.get_positions_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		const auto seed = Random::randsize_t(0, 99999);

		// Evaluate each cell individually
		for (const auto& spawn_pos : floor_cells)
		{
			// Allow only one entity per cell
			if (!ECS::get_entities(registry, spawn_pos).empty())
				continue;

			// Randomize order each cell
			std::shuffle(id_pool.begin(), id_pool.end(), Random::rng());

			// Check each entity and their spawn conditions
			for (const auto& id : id_pool)
			{
				const auto& spawn_data = LUT.at(id)["spawn"];
				if (spawn_data.contains("perlin"))
				{
					const auto& perlin = spawn_data["perlin"];
					const auto frequency = perlin["frequency"].get<double>();
					const auto treshold = perlin["treshold"].get<double>();
					const auto octaves = perlin.contains("octaves") ?
						perlin["octaves"].get<size_t>() : 1;
					const auto noise = Random::noise2D(
							spawn_pos.cell_idx / cave.get_size(),
							spawn_pos.cell_idx % cave.get_size(),
							frequency, octaves, seed);
					if (noise < treshold)
					{
						continue;
					}
				}
				if (spawn_data.contains("space"))
				{
					const auto& space = spawn_data["space"];
					if (!space.contains("radius") || !space.contains("neighbors"))
						Log::error(id + ": Space missing data: " + spawn_data.dump(4));
					const auto radius = space["radius"].get<double>();
					const auto& [min_neighbors, max_neighbors] = Parser::parse_range(space["neighbors"]);
					size_t neighbors = cave.get_nearby_positions(spawn_pos, radius, Cell::Type::Rock).size();
					for (const auto nearby_pos : cave.get_nearby_positions(spawn_pos, radius, Cell::Type::Floor))
						neighbors += ECS::get_entities(registry, nearby_pos).size();
					if (neighbors < min_neighbors || neighbors > max_neighbors)
						continue;
				}
				if (spawn_data.contains("water"))
				{
					const auto& water = spawn_data["water"];
					const auto radius = water["radius"].get<double>();
					const auto& [min, max] = Parser::parse_range(water["amount"]);
					double liquid_volume = cave.get_cell(spawn_pos)
						.get_liquid_mixture()
						.get_volume(Liquid::Type::Water);
					for (const auto nearby_pos : cave.get_nearby_positions(spawn_pos, radius, Cell::Type::Floor))
					{
						liquid_volume += cave.get_cell(nearby_pos)
							.get_liquid_mixture()
							.get_volume(Liquid::Type::Water);
					}
					if (liquid_volume < min || liquid_volume > max)
					{
						continue;
					}
				}
				if (spawn_data.contains("light"))
				{
					const auto& light = spawn_data["light"];
					const auto radius = light["radius"].get<double>();
					const auto& [min, max] = Parser::parse_range(light["amount"]);
					double illumination = LightingSystem::get_illumination(cave.get_cell(spawn_pos));
					for (const auto nearby_pos : cave.get_nearby_positions(spawn_pos, radius))
						illumination += LightingSystem::get_illumination(cave.get_cell(nearby_pos));
					if (illumination < min || illumination > max)
					{
						continue;
					}
				}
				EntityFactory::instance().create_entity(registry, id, spawn_pos);
				break;
			}
		}
	}
};
