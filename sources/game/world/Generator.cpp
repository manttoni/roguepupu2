#include "game/world/Generator.hpp"
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
			const GlobalPosition& position)
	{
		const auto& layers = conf.at("layers");
		const auto& layer = layers.at(id);
		if (layer.contains("terrain"))
		{
			const auto& terrain = layer.at("terrain");
			if (terrain.contains("material"))
			{
				const auto& material = terrain.at("material").get<std::string>();
				cell.material = Game::Enum::from_string<Material>(material);
			}
			if (terrain.contains("form"))
			{
				const auto& form = terrain.at("form").get<std::string>();
				cell.form = Game::Enum::from_string<Form>(form);
			}
		}
		if (layer.contains("entities"))
		{
			const auto& entities = layer.at("entities");
			for (const auto& entity : entities)
			{
				const auto chance = entity.value("chance", 1.0);
				// TODO: make deterministic
				if (Random::rand<double>(0.0, 1.0) > chance)
					continue;
				const auto entity_id = entity.at("id").get<std::string>();
				spawn_requests.push_back(SpawnRequest{entity_id, position});
			}
		}
		if (layer.contains("noise") && layer.at("noise").contains("regions"))
		{
			const auto& noise = layer.at("noise");
			const Random::Perlin::Settings settings{
				.frequency = noise.value("frequency", 0.01),
					.octaves = noise.value("octaves", 1),
					.persistence = noise.value("persistence", 0.5)
			};
			const auto noise_value =
				Random::Perlin::Generator{seed, settings}.noise2(position);

			for (const auto& region : noise.at("regions"))
			{
				if (!region.contains("id") || !region.contains("range"))
					continue;
				const auto& id = region.at("id").get<std::string>();
				const auto& range = region.at("range").get<std::vector<double>>();
				if (id.empty() || noise_value < range[0] || noise_value > range[1])
					continue;
				apply_layer(id, cell, position);
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
			apply_layer(id, cell, position);
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
				const GlobalPosition global_position = to_global(chunk_position, local_position);
				chunk.get_cell(local_position) = generate_cell(global_position);
			}
		}
		return chunk;
	}
}
