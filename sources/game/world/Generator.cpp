#include "game/world/Generator.hpp"
#include "utils/IO.hpp"

namespace Game::World
{
	Generator::Generator(
			const std::string& seed,
			const std::filesystem::path& path)
	{
		const auto conf = IO::read_json(path);
		if (conf.contains("default_terrain"))
		{
			default_terrain =
				Enum::from_string<Enum::Terrain>(
						conf.at("default_terrain").get<std::string>());
		}
		if (conf.contains("layers") && conf.at("layers").is_array())
		{
			for (const auto& layer : conf.at("layers"))
			{
				const auto terrain =
					Enum::from_string<Enum::Terrain>(
							layer.at("terrain").get<std::string>());
				const auto frequency =
					layer.at("frequency").get<double>();
				const auto octaves =
					layer.at("octaves").get<std::int32_t>();
				const auto persistence =
					layer.at("persistence").get<double>();
				auto perlin =
					Random::Perlin::Generator(
							seed, {frequency, octaves, persistence});
				const auto threshold =
					layer.at("threshold").get<double>();
				layers.emplace_back(
						terrain,
						perlin,
						threshold
						);
			}
		}
	}

	Chunk Generator::generate_chunk(const ChunkPosition& chunk_position) const
	{
		Chunk chunk;
		for (int y = 0; y < Chunk::height; ++y)
		{
			for (int x = 0; x < Chunk::width; ++x)
			{
				const LocalPosition local_position{y, x};
				const GlobalPosition global_position = to_global(chunk_position, local_position);
				auto& cell = chunk.get_cell(local_position);
				cell.terrain = default_terrain;

				for (const auto& layer : layers)
				{
					if (layer.perlin.noise2(global_position.vec2())
							<= layer.threshold)
					{
						cell.terrain = layer.terrain;
					}
				}
			}
		}
		return chunk;
	}
}
