#pragma once

#include <charconv>
#include <utility>
#include <filesystem>
#include "ncurses/Color.hpp"
#include "domain/Dice.hpp"
#include "nlohmann/json.hpp"
#include "utils/Error.hpp"
#include "utils/Random.hpp"
#include "utils/Range.hpp"
#include "utils/JsonUtils.hpp"

namespace CaveGenerator { struct Data; };
namespace Parser
{
	using Json = nlohmann::json;

	Ncurses::Color parse_color(const Json& data);
	Json read_json_file(const std::filesystem::path& path);
	void parse_cave_generation_conf(const std::string& conf_id, CaveGenerator::Data& data);
	void parse_cave_generation_conf(const Json& conf, CaveGenerator::Data& data);
	Random::Perlin parse_perlin(const Json& data);
	Domain::Dice parse_dice(const Json& data);

	template<typename T>
		Range<T> parse_range(const Json& data)
		{
			if (data.is_number())
			{
				const T n = data.get<T>();
				return {n, n};
			}
			else if (data.is_string())
			{
				const auto str = data.get<std::string>();
				const auto pos = str.find('-');
				if (pos == std::string::npos)
					Error::fatal("Invalid range: " + str);

				T min{}, max{};
				auto [p1, ec1] = std::from_chars(str.data(), str.data() + pos, min);
				auto [p2, ec2] = std::from_chars(str.data() + pos + 1, str.data() + str.size(), max);

				if (ec1 != std::errc{} || ec2 != std::errc{} || min > max)
					Error::fatal("Invalid range: " + str);

				return { min, max };
			}
			else if (data.is_array())
			{
				const T min = data[0].get<T>();
				const T max = data[1].get<T>();
				if (min > max)
					Error::fatal("Invalid range: " + data.dump(4));
				return { min, max };
			}
			else if (data.contains("range"))
				return parse_range<T>(data["range"]);
			Error::fatal("Unsupported range format: " + data.dump(4));
		}
};

