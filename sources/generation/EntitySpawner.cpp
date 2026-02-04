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
	/* Spawn in clusters where perlin noise values are greater than treshold
	 * */
	bool evaluate_perlin(const entt::registry& registry, const Position& spawn_pos, const nlohmann::json& perlin)
	{
		if (!perlin.contains("frequency") || !perlin.contains("treshold"))
			Error::fatal("Perlin is missing data: " + perlin.dump(4));
		const auto& cave = ECS::get_cave(registry, spawn_pos.cave_idx);
		const auto frequency = perlin["frequency"].get<double>();
		const auto treshold = perlin["treshold"].get<double>();
		const auto octaves = perlin.contains("octaves") ?
			perlin["octaves"].get<size_t>() : 1;
		const auto noise = Random::noise2D(
				spawn_pos.cell_idx / cave.get_size(),
				spawn_pos.cell_idx % cave.get_size(),
				frequency, octaves, spawn_pos.cave_idx
				);
		return noise >= treshold;
	}

	/* Spawn if there is the right amount of entities/blockers/space within an area around.
	 * Should not consider what counts as "empty" cell or solid blocker too much.
	 * */
	bool evaluate_space(const entt::registry& registry, const Position& spawn_pos, const nlohmann::json& space)
	{
		if (!space.contains("radius"))
			Error::fatal("Space missing data: " + space.dump(4));
		const auto radius = space["radius"].get<double>();
		const auto& cave = ECS::get_cave(registry, spawn_pos.cave_idx);
		if (space.contains("empty"))
		{
			size_t empty = 0;
			for (const auto& pos : cave.get_nearby_positions(spawn_pos, radius, Cell::Type::Floor))
			{
				if (ECS::get_entities(registry, pos).size() == 0)
					empty++;
			}
			const auto& [min_empty, max_empty] = Parser::parse_range<size_t>(space["empty"]);
			if (empty < min_empty || empty > max_empty)
				return false;
		}
		else if (space.contains("solid")) // Rock counts as solid
		{
			size_t solid = 0;
			for (const auto& pos : cave.get_nearby_positions(spawn_pos, radius))
			{
				if (cave.get_cell(pos).get_type() == Cell::Type::Rock)
					solid++;
				solid += ECS::get_entities(registry, pos, Solid(true)).size();
			}
			const auto& [min_solid, max_solid] = Parser::parse_range<size_t>(space["solid"]);
			if (solid < min_solid || solid > max_solid)
				return false;
		}
		else
			Error::fatal("Space needs more data: " + space.dump(4));
		return true;
	}

	/* Some entities require liquid amount and type to be just right
	 * */
	bool evaluate_liquid(const entt::registry& registry, const Position& spawn_pos, const nlohmann::json& liquid)
	{
		if (!liquid.contains("radius") || !liquid.contains("type") || !liquid.contains("amount"))
			Error::fatal("Liquid needs more data: " + liquid.dump(4));
		const auto& cave = ECS::get_cave(registry, spawn_pos.cave_idx);
		const auto radius = liquid["radius"].get<double>();
		const Liquid::Type type = Liquid::from_string(liquid["type"].get<std::string>());
		const auto& [min, max] = Parser::parse_range<double>(liquid["amount"]);
		double liquid_volume = cave.get_cell(spawn_pos)
			.get_liquid_mixture()
			.get_volume(type);
		for (const auto nearby_pos : cave.get_nearby_positions(spawn_pos, radius, Cell::Type::Floor))
		{
			liquid_volume += cave.get_cell(nearby_pos)
				.get_liquid_mixture()
				.get_volume(type);
		}
		if (liquid_volume < min || liquid_volume > max)
			return false;
		return true;
	}

	/* Require average light level to be some amount within a radius
	 * */
	bool evaluate_light(const entt::registry& registry, const Position& spawn_pos, const nlohmann::json& light)
	{
		if (!light.contains("radius") || !light.contains("amount"))
			Error::fatal("Light needs more data: " + light.dump(4));
		const auto& cave = ECS::get_cave(registry, spawn_pos.cave_idx);
		const auto radius = light["radius"].get<double>();
		const auto& [min, max] = Parser::parse_range<double>(light["amount"]);
		double avg = LightingSystem::get_illumination(cave.get_cell(spawn_pos));
		const auto nearby = cave.get_nearby_positions(spawn_pos, radius);
		for (const auto nearby_pos : nearby)
			avg += LightingSystem::get_illumination(cave.get_cell(nearby_pos));
		avg /= 1 + nearby.size();
		if (avg < min || avg > max)
			return false;
		return true;
	}

	void spawn_entities(entt::registry& registry, const size_t cave_idx, nlohmann::json filter)
	{
		filter["spawn"] = "any";
		const auto& cave = ECS::get_cave(registry, cave_idx);
		auto id_pool = EntityFactory::instance().filter_entity_ids(filter);
		assert(id_pool.size() > 0);
		const auto& LUT = EntityFactory::instance().get_LUT();
		auto floor_cells = cave.get_positions_with_type(Cell::Type::Floor);
		std::shuffle(floor_cells.begin(), floor_cells.end(), Random::rng());
		for (const auto& spawn_pos : floor_cells)
		{
			if (!ECS::get_entities(registry, spawn_pos).empty())
				continue;
			std::shuffle(id_pool.begin(), id_pool.end(), Random::rng());
			for (const auto& id : id_pool)
			{
				const auto& spawn_data = LUT.at(id)["spawn"];
				if (spawn_data.contains("perlin") && evaluate_perlin(registry, spawn_pos, spawn_data["perlin"]) == false)
					continue;
				if (spawn_data.contains("space") && evaluate_space(registry, spawn_pos, spawn_data["space"]) == false)
					continue;
				if (spawn_data.contains("liquid") && evaluate_liquid(registry, spawn_pos, spawn_data["liquid"]) == false)
					continue;
				if (spawn_data.contains("light") && evaluate_light(registry, spawn_pos, spawn_data["light"]) == false)
					continue;
				EntityFactory::instance().create_entity(registry, id, spawn_pos);
				break;
			}
		}
	}

	void despawn_entities(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		auto floor = cave.get_positions_with_type(Cell::Type::Floor);
		std::shuffle(floor.begin(), floor.end(), Random::rng());
		const auto& LUT = EntityFactory::instance().get_LUT();
		for (const auto& pos : floor)
		{
			for (const auto entity : ECS::get_entities(registry, pos))
			{
				const auto& name = registry.get<Name>(entity).name;
				if (!LUT.contains(name) || !LUT.at(name).contains("spawn"))
					continue;
				const auto& spawn_data = LUT.at(name)["spawn"];
				if ((spawn_data.contains("space") && evaluate_space(registry, pos, spawn_data["space"]) == false) ||
					(spawn_data.contains("liquid") && evaluate_liquid(registry, pos, spawn_data["liquid"]) == false) ||
					(spawn_data.contains("light") && evaluate_light(registry, pos, spawn_data["light"]) == false))
					registry.destroy(entity);
			}
		}
	}
};
