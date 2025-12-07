#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <vector>
#include "entt.hpp"

class Cell;
class EntityFactory
{
	private:
		std::unordered_map<std::string, nlohmann::json> LUT;

		bool exclude(const nlohmann::json& data, const nlohmann::json& filter);

	public:
		static EntityFactory& instance()
		{
			static EntityFactory ef;
			return ef;
		}
		void init();
		void read_definitions(const std::filesystem::path& path);
		void add_entities(nlohmann::json& json, const std::string& category, const std::string& subcategory);
		entt::entity create_entity(entt::registry& registry, const std::string& name, Cell* cell = nullptr);
		std::vector<std::string> random_pool(const nlohmann::json& filter, const size_t amount);
};
