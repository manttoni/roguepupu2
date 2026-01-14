#pragma once

#include <filesystem>             // for path
#include <nlohmann/json.hpp>  // for json
#include <string>                 // for string
#include "entt.hpp"               // for entity, registry, size_t, unordered...
#include "Components.hpp"
class Cell;  // lines 10-10

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
		entt::entity create_entity(entt::registry& registry, const std::string& name, const std::optional<Position>& position = std::nullopt);
		std::vector<std::string> random_pool(const nlohmann::json& filter, const size_t amount = SIZE_MAX);
		const std::unordered_map<std::string, nlohmann::json>& get_LUT() const { return LUT; }
		std::vector<std::string> get_category_names() const;
		std::vector<std::string> get_subcategory_names(const std::string& category) const;
};
