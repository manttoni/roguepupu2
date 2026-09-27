#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <filesystem>

using Json = nlohmann::json;

struct EntityDatabase
{
	static constexpr const char* default_entities = "data/entities.json";

	std::unordered_map<std::string, Json> definitions;

	explicit EntityDatabase(const std::filesystem::path& path);
	EntityDatabase() : EntityDatabase(default_entities) {}
};
