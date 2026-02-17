#pragma once

#include <charconv>
#include <utility>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "domain/Damage.hpp"
#include "domain/Color.hpp"
#include "domain/Effect.hpp"
#include "domain/Conditions.hpp"
#include "utils/Error.hpp"
#include "domain/LootTable.hpp"
#include "utils/Range.hpp"

namespace CaveGenerator { struct Data; };
namespace Parser
{
	Color parse_color(const nlohmann::json& data);
	Effect parse_effect(const nlohmann::json& data);
	Conditions parse_conditions(const nlohmann::json& data);
	nlohmann::json read_json_file(const std::filesystem::path& path);
	void parse_cave_generation_conf(const std::string& conf_id, CaveGenerator::Data& data);
	LootTable parse_loot_table(const nlohmann::json& data);
	Damage::Spec parse_damage_spec(const nlohmann::json& data);

	template<typename T>
		Range<T> parse_range(const nlohmann::json& data)
		{
			if (data.is_number())
			{
				const T n = data.get<T>();
				return {n, n};
			}
			if (data.is_string())
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
			Error::fatal("Unsupported range format: " + data.dump(4));
		}
};

