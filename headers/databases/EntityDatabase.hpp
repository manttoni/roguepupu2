#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <filesystem>

using Json = nlohmann::json;

struct EntityDatabase
{
	static constexpr const char* entity_definitions_path = "data/entity/definitions.json";

	std::unordered_map<std::string, Json> definitions;

	explicit EntityDatabase(const std::filesystem::path& path);
	EntityDatabase() : EntityDatabase(entity_definitions_path) {}
};
