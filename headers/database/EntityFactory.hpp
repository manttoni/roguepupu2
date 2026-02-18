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

	public:
		static EntityFactory& instance()
		{
			static EntityFactory ef;
			return ef;
		}
		EntityFactory() { if (LUT.empty()) init(); }
		const std::unordered_map<std::string, nlohmann::json>& get_LUT() const { return LUT; }
		entt::entity create_entity(entt::registry& registry, const std::string& name, const std::optional<Position>& position = std::nullopt) const;
		std::vector<entt::entity> create_entities(entt::registry& registry, const nlohmann::json& include = {}, const nlohmann::json& exclude = {}) const;
		std::vector<entt::entity> create_entities(entt::registry& registry, const std::vector<std::string>& entity_ids) const;
		std::vector<entt::entity> create_entities(entt::registry& registry, const std::string& id, const size_t amount) const;
		std::vector<std::string> filter_entity_ids(const nlohmann::json& include = {}, const nlohmann::json& exclude = {}) const;
		void add_entities(nlohmann::json& json);
		void init();
		void clear() { LUT.clear(); }
};
