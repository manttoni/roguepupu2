#include <stdexcept>
#include "game/world/Generator.hpp"
#include "utils/Perlin.hpp"
#include "utils/Random.hpp"
#include "utils/IO.hpp"
#include "game/world/Chunk.hpp"
#include "utils/Log.hpp"

namespace Game::World
{
	Generator::Generator(
			const std::string& seed,
			const std::filesystem::path& path) :
		seed(seed),
		conf(IO::read_json(path))
	{}

	void Generator::apply_layer(
			const std::string& id,
			Cell& cell,
			const GlobalPosition& position,
			const double weight)
	{
		const auto& layers = conf.at("layers");
		const auto& layer = layers.at(id);
		if (layer.contains("terrain"))
		{
			const auto& terrain = layer.at("terrain");
			if (terrain.contains("material") && roll(weight))
			{
				const auto& material =
					terrain.at("material").get<std::string>();
				cell.material = Enum::from_string<Enum::Material>(material);
			}
			if (terrain.contains("form") && roll(weight))
			{
				const auto& form = terrain.at("form").get<std::string>();
				cell.form = Enum::from_string<Enum::Form>(form);
			}
		}
		if (layer.contains("entities"))
		{
			const auto& entities = layer.at("entities");
			for (const auto& entity : entities)
			{
				if (!roll(weight))
					continue;

				const auto chance = entity.value("chance", 1.0);
				if (!roll(chance))
					continue;
				const auto entity_id = entity.at("id").get<std::string>();
				Entity::SpawnRequest request;
				request.id = entity_id;
				request.position = position;
				spawn_requests.push_back(request);
			}
		}
		if (layer.contains("noise"))
		{
			if (!layer.at("noise").contains("regions"))
			{
				Log::warning()
					<< "noise has no regions, which makes it useless";
				return;
			}

			const auto& noise = layer.at("noise");
			const Random::Perlin::Settings settings{
				.frequency = noise.value("frequency", 0.01),
					.octaves = noise.value("octaves", 1),
					.persistence = noise.value("persistence", 0.5)
			};
			const double noise_value =
				Random::Perlin::Generator{
					seed + id,
					settings}.noise2(position.vec2());

			// Give each region a weight
			std::map<std::string, double> weights;

			for (const auto& region : noise.at("regions"))
			{
				if (!region.contains("id") || !region.contains("range"))
					continue;

				const auto id = region.at("id").get<std::string>();
				const auto range =
					region.at("range").get<std::vector<double>>();

				if (id.empty())
					continue;

				if (range.size() != 2 || !(range[0] < range[1]))
				{
					Log::warning() << "Invalid noise range for region: " << id;
					continue;
				}

				const auto min = range[0];
				const auto max = range[1];

				if (noise_value < min || noise_value > max)
					continue;

				const auto midpoint = (min + max) / 2.0;
				const auto half_width = (max - min) / 2.0;

				const auto weight = std::max(
						0.0,
						1.0 - std::abs(noise_value - midpoint) / half_width);

				if (weight > 0.0)
					weights[id] = weight;
			}

			double total_weight = 0.0;
			for (const auto& [id, weight] : weights)
				total_weight += weight;

			if (total_weight > 0.0)
			{
				for (auto& [id, weight] : weights)
					weight = 100.0 * weight / total_weight;
			}

			for (const auto& region : noise.at("regions"))
			{
				const auto id = region.at("id").get<std::string>();
				apply_layer(
						id,
						cell,
						position,
						weights[id]);
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

		if (cell.material == Enum::Material::None
				|| cell.form == Enum::Form::None)
		{
			throw std::runtime_error(
					std::string("Generated invalid cell: ")
					+ Cell::to_string(cell));
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

	bool Generator::roll(const double chance) const
	{
		// TODO: make this deterministic
		return Random::roll(chance);
	}
}
