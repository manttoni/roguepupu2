#include <algorithm>
#include <vector>
#include <map>
#include "Parser.hpp"
#include "systems/LightingSystem.hpp"
#include "ECS.hpp"
#include "systems/PositionSystem.hpp"
#include "Components.hpp"
#include "entt.hpp"
#include "Cave.hpp"
#include "nlohmann/json.hpp"
#include "EntityFactory.hpp"
#include "Utils.hpp"

namespace EntitySpawner
{
	void spawn_entities(entt::registry& registry, const size_t cave_idx)
	{
		const auto& cave = PositionSystem::get_cave(registry, cave_idx);
		nlohmann::json filter = {{"spawn", "any"}, {"player"}, {"none"}};
		auto id_pool = EntityFactory::instance().random_pool(filter);
		const auto& LUT = EntityFactory::instance().get_LUT();
		auto floor_cells = cave.get_cells_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		const auto seed = Random::randsize_t(0, 99999);
		for (const auto spawn_idx : floor_cells)
		{
			const Position spawn_pos = {.cell_idx = spawn_idx, .cave_idx = cave.get_idx()};
			if (!ECS::get_entities(registry, spawn_pos).empty())
				continue;
			std::shuffle(id_pool.begin(), id_pool.end(), Random::rng());
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
							spawn_idx / cave.get_size(), spawn_idx % cave.get_idx(),
							frequency, octaves, seed);
					if (noise < treshold)
						continue;
				}
				if (spawn_data.contains("space"))
				{
					const auto& space = spawn_data["space"];
					const auto radius = space["radius"].get<double>();
					const auto& [min_blockers, max_blockers] = Parser::parse_range(space["blockers"]);
					size_t blockers = cave.get_nearby_ids(spawn_idx, radius, Cell::Type::Rock).size();
					for (const auto nearby_idx : cave.get_nearby_ids(spawn_idx, radius, Cell::Type::Floor))
					{
						const auto entities = ECS::get_entities(registry, Position{.cell_idx = nearby_idx, .cave_idx = cave.get_idx()});
							for (const auto entity : entities)
							{
								if (ECS::get_size(registry, entity) > 0.5)
									blockers++;
							}
						}
						if (blockers < min_blockers || blockers > max_blockers)
							continue;
					}
					if (spawn_data.contains("water"))
					{
						const auto& water = spawn_data["water"];
						const auto radius = water["radius"].get<double>();
						const auto& [min, max] = Parser::parse_range(water["amount"]);
						double liquid_volume = cave.get_cell(spawn_idx)
							.get_liquid_mixture()
							.get_volume(Liquid::Type::Water);
						for (const auto nearby_idx : cave.get_nearby_ids(spawn_idx, radius, Cell::Type::Floor))
						{
							liquid_volume += cave.get_cell(nearby_idx)
								.get_liquid_mixture()
								.get_volume(Liquid::Type::Water);
						}
						if (liquid_volume < min || liquid_volume > max)
							continue;
					}
					if (spawn_data.contains("light"))
					{
						const auto& light = spawn_data["light"];
						const auto radius = light["radius"].get<double>();
						const auto& [min, max] = Parser::parse_range(light["amount"]);
						double illumination = LightingSystem::get_illumination(cave.get_cell(spawn_idx));
						for (const auto nearby_idx : cave.get_nearby_ids(spawn_idx, radius))
							illumination += LightingSystem::get_illumination(cave.get_cell(nearby_idx));
						if (illumination < min || illumination > max)
							continue;
					}
					EntityFactory::instance().create_entity(registry, id, spawn_pos);
					break;
				}
			}
		}
	};
