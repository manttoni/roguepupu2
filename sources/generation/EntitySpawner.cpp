#include <stddef.h>
#include <nlohmann/json_fwd.hpp>
#include <algorithm>
#include <vector>
#include <map>
#include <cassert>
#include <optional>
#include <string>
#include <unordered_map>

#include "utils/Parser.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "components/Components.hpp"
#include "domain/Cave.hpp"
#include "nlohmann/json.hpp"
#include "database/EntityFactory.hpp"
#include "utils/Random.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "domain/Cell.hpp"
#include "domain/Liquid.hpp"
#include "domain/LiquidMixture.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"

namespace EntitySpawner
{
	/* Entities that can spawn will spawn when calling this.
	 * So far the only way is to have SpawnsNaturally tag
	 * */
	void spawn_entities(entt::registry& registry, const size_t cave_idx)
	{
		// get spawn points
		const auto& cave = ECS::get_cave(registry, cave_idx);
		auto floor_cells = cave.get_positions_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());

		// get entity ids/names and reference to all entity data (look up table)
		const nlohmann::json filter = {
			{ "contains_all", {{ "tags", {"spawns_naturally"}}}}
		};
		auto ids = EntityFactory::instance().filter_entity_ids(filter);
		const auto& LUT = EntityFactory::instance().get_LUT();

		assert(!ids.empty());

		size_t spawned = 0;
		for (const auto& spawn_pos : floor_cells)
		{
			if (!ECS::get_entities(registry, spawn_pos).empty())
				continue;
			std::shuffle(ids.begin(), ids.end(), Random::rng());
			for (const auto& id : ids)
			{
				const auto& entity_data = LUT.at(id);

				const auto& spawn_chance = entity_data["spawn_chance"];
				const double chance = spawn_chance["chance"].get<double>();
				if (!Random::roll(chance))
					continue;
				Random::Perlin perlin = Parser::parse_perlin(spawn_chance["perlin"]);
				perlin.seed = cave_idx ^ std::hash<std::string>{}(id);
				if (perlin.enabled && !Random::roll(perlin, Vec2(spawn_pos.cell_idx, cave.get_size())))
					continue;

				const auto& environment_sensitive = entity_data["environment_sensitive"];
				const auto photo = environment_sensitive["photo"].get<std::string>();
				const auto hydro = environment_sensitive["hydro"].get<std::string>();
				const bool has_light = ECS::get_light_amount(registry, spawn_pos) > 0;
				const bool has_water = ECS::get_liquid_amount(registry, spawn_pos, Liquid::Type::Water) > 0;
				if (photo == "philia" && !has_light) continue;
				if (photo == "phobia" && has_light) continue;
				if (hydro == "philia" && !has_water) continue;
				if (hydro == "phobia" && has_water) continue;

				const auto& spawn_position = entity_data["spawn_position"];
				const auto solid_neighbors_range = Parser::parse_range<size_t>(spawn_position["solid_neighbors"]);
				const auto rock_neighbors_range = Parser::parse_range<size_t>(spawn_position["rock_neighbors"]);
				size_t solid_neighbors = 0;
				size_t rock_neighbors = 0;
				for (const auto& pos : cave.get_nearby_positions(spawn_pos, 1.5))
				{
					if (ECS::is_solid(registry, pos))
						solid_neighbors++;
					if (ECS::get_cell(registry, pos).get_type() == Cell::Type::Rock)
						rock_neighbors++;
				}
				if (!solid_neighbors_range.contains(solid_neighbors) ||
					!rock_neighbors_range.contains(rock_neighbors))
					continue;

				EntityFactory::instance().create_entity(registry, id, spawn_pos);
				spawned++;
				break;
			}
		}

		Log::log("Spawned " + std::to_string(spawned) + " entities");
	}
};
