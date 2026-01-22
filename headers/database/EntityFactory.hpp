#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>
#include <cstddef>
#include "domain/Position.hpp"
#include "external/entt/fwd.hpp"
#include "domain/LiquidMixture.hpp"

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
		EntityFactory() { init(); }
		const std::unordered_map<std::string, nlohmann::json>& get_LUT() const { return LUT; }
		entt::entity create_entity(entt::registry& registry, const std::string& name, const std::optional<Position>& position = std::nullopt);
		std::vector<std::string> get_category_names() const;
		std::vector<std::string> get_subcategory_names(const std::string& category) const;
		std::vector<std::string> filter_entity_ids(const nlohmann::json& filter, const size_t amount = SIZE_MAX); // is amount ever used?
		void add_entities(nlohmann::json& json, const std::string& category, const std::string& subcategory);
		void init();
		void read_definitions(const std::filesystem::path& path);
		entt::entity create_potion(entt::registry& registry, const LiquidMixture& lm);
};
