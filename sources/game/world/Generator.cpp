#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "game/world/Generator.hpp"
#include "game/world/Chunk.hpp"
#include "utils/Perlin.hpp"
#include "utils/Random.hpp"
#include "utils/IO.hpp"
#include "utils/Log.hpp"

namespace Game::World
{
	Generator::Generator(
			const std::string& seed,
			const std::filesystem::path& path) :
		seed(seed),
		seed_hash(Random::hash_seed(seed)),
		conf(IO::read_json(path))
	{}

	void Generator::apply_layer(
			const std::string& id,
			Cell& cell,
			const GlobalPosition& position,
			const double weight)
	{
		if (weight <= 0.0)
			return;

		const auto& layers = conf.at("layers");
		const auto& layer = layers.at(id);

		// Each layer and purpose gets its own deterministic roll.
		const auto layer_seed = seed_hash ^ Random::hash_seed(id);
		const auto roll = [&](const double chance, const std::string_view stream)
		{
			return Random::at(
					layer_seed,
					position,
					Random::hash_seed(stream)) < chance;
		};

		if (layer.contains("terrain"))
		{
			const auto& terrain = layer.at("terrain");

			if (terrain.contains("material") &&
					(cell.material == Enum::Material::None ||
					 roll(weight, "terrain/material")))
			{
				const auto material =
					terrain.at("material").get<std::string>();
				cell.material = Enum::from_string<Enum::Material>(material);
			}

			if (terrain.contains("form") &&
					(cell.form == Enum::Form::None ||
					 roll(weight, "terrain/form")))
			{
				const auto form =
					terrain.at("form").get<std::string>();
				cell.form = Enum::from_string<Enum::Form>(form);
			}
		}

		if (layer.contains("entities"))
		{
			const auto& entities = layer.at("entities");

			for (std::size_t index = 0; index < entities.size(); ++index)
			{
				const auto& entity = entities.at(index);
				const auto stream = std::string("entity/") + std::to_string(index);

				if (!roll(weight, stream + "/weight"))
					continue;

				const auto chance = entity.value("chance", 1.0);
				if (!roll(chance, stream + "/chance"))
					continue;

				Entity::SpawnRequest request;
				request.id = entity.at("id").get<std::string>();
				request.position = position;
				spawn_requests.push_back(request);
				Log::debug() << "Requesting " << request.id << " at " <<
					request.position;
			}
		}

		if (layer.contains("noise"))
		{
			const auto& noise = layer.at("noise");

			if (!noise.contains("regions"))
			{
				Log::warning()
					<< "noise has no regions, which makes it useless";
				return;
			}

			const Random::Perlin::Settings settings{
				.frequency = noise.value("frequency", 0.01),
				.octaves = noise.value("octaves", 1),
				.persistence = noise.value("persistence", 0.5)
			};

			const double noise_value =
				Random::Perlin::Generator{
					seed + id,
					settings}.noise2(position.vec2());

			std::map<std::string, double> weights;

			for (const auto& region : noise.at("regions"))
			{
				if (!region.contains("id") || !region.contains("range"))
					continue;

				const auto region_id = region.at("id").get<std::string>();
				const auto range =
					region.at("range").get<std::vector<double>>();

				if (region_id.empty())
					continue;

				if (range.size() != 2 || !(range[0] < range[1]))
				{
					Log::warning()
						<< "Invalid noise range for region: " << region_id;
					continue;
				}

				const auto min = range[0];
				const auto max = range[1];

				if (noise_value < min || noise_value > max)
					continue;

				const auto midpoint = (min + max) / 2.0;
				const auto half_width = (max - min) / 2.0;

				const auto region_weight = std::max(
						0.0,
						1.0 - std::abs(noise_value - midpoint) / half_width);

				if (region_weight > 0.0)
					weights[region_id] = region_weight;
			}

			double total_weight = 0.0;
			for (const auto& [region_id, region_weight] : weights)
				total_weight += region_weight;

			if (total_weight > 0.0)
			{
				for (auto& [region_id, region_weight] : weights)
					region_weight /= total_weight;
			}

			// Preserve the region order from the configuration.
			for (const auto& region : noise.at("regions"))
			{
				if (!region.contains("id"))
					continue;

				const auto region_id = region.at("id").get<std::string>();
				const auto found = weights.find(region_id);
				if (found == weights.end())
					continue;

				apply_layer(region_id, cell, position, found->second);
			}
		}
	}

	Cell Generator::generate_cell(const GlobalPosition& position)
	{
		Cell cell;

		const auto& bases = conf.at("bases");
		const auto& layers = conf.at("layers");

		for (const auto& base : bases)
		{
			const auto id = base.get<std::string>();

			if (layers.contains(id))
				apply_layer(id, cell, position, 1.0);
		}

		if (cell.material == Enum::Material::None ||
				cell.form == Enum::Form::None)
		{
			throw std::runtime_error(
					std::string("Generated invalid cell: ") +
					Cell::to_string(cell));
		}

		return cell;
	}

	Chunk Generator::generate_chunk(const ChunkPosition& chunk_position)
	{
		Chunk chunk;

		for (int y = 0; y < Chunk::height; ++y)
		{
			for (int x = 0; x < Chunk::width; ++x)
			{
				const LocalPosition local_position{y, x};
				const GlobalPosition global_position =
					to_global(chunk_position, local_position);

				chunk.get_cell(local_position) = generate_cell(global_position);
			}
		}

		Log::debug()
			<< "Chunk generated: " << chunk
			<< ", at: " << chunk_position
			<< ", spawns: " << spawn_requests.size();

		return chunk;
	}
}


