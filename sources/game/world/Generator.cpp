#include "game/world/Generator.hpp"
#include "utils/IO.hpp"
#include "game/world/Chunk.hpp"
#include "utils/Log.hpp"

namespace Game::World
{
	Generator::Generator(
			const std::string& seed,
			const std::filesystem::path& path)
		: seed(seed)
	{
		const auto conf = IO::read_json(path);
		if (conf.contains("layers") && conf.at("layers").is_array())
			parse_layers(conf.at("layers"));
	}

	void Generator::parse_layers(const Json& layers)
	{
		for (const auto& layer : layers)
		{
			parse_layer(layer);
		}
	}

	void Generator::parse_layer(const Json& layer)
	{
		if (layer.value("disabled", false))
			return;

		const Random::Perlin::Settings perlin_settings{
			.frequency = layer.value("frequency", 1.0),
				.octaves = layer.value("octaves", std::int32_t{1}),
				.persistence = layer.value("persistence", 1.0)
		};

		layers.emplace_back(Layer{
				.material = Enum::from_string<Enum::Material>(layer.value("material", "Stone")),
				.form = Enum::from_string<Enum::Form>(layer.value("form", "Floor")),
				.threshold = layer.value("threshold", 1.0),
				.water_depth = layer.value("water_depth", 0.0),
				.id = layer.value("id", "none"),
				.perlin = Random::Perlin::Generator(seed, perlin_settings)
				});
	}

	Chunk Generator::generate_chunk(const ChunkPosition& chunk_position) const
	{
		Chunk chunk;
		for (int y = 0; y < Chunk::height; ++y)
		{
			for (int x = 0; x < Chunk::width; ++x)
			{
				const LocalPosition local_position{y, x};
				const GlobalPosition global_position =
					to_global(chunk_position, local_position);
				auto& cell = chunk.get_cell(local_position);

				for (const auto& layer : layers)
				{
					const auto p = layer.perlin.noise2(global_position.vec2());
					if (p <= layer.threshold)
					{
						cell.material = layer.material;
						cell.form = layer.form;
						cell.water_depth += layer.water_depth;
					}
				}
			}
		}
		return chunk;
	}
}
