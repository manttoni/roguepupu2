#pragma once

#include <utility>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "domain/Effect.hpp"
#include "domain/Conditions.hpp"

namespace CaveGenerator { struct Data; };
namespace Parser
{
	Effect parse_effect(const nlohmann::json& data);
	Conditions parse_conditions(const nlohmann::json& data);
	std::pair<size_t, size_t> parse_range(const nlohmann::json& data);
	nlohmann::json read_file(const std::filesystem::path& path);
	void parse_cave_generation_conf(const std::string& conf, CaveGenerator::Data& data);
};

