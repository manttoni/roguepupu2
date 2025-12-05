#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include "entt.hpp"
#include "Cell.hpp"

class EntityFactory
{
	private:
		static constexpr const char* items_path = "data/items.json";
		static constexpr const char* fungi_path = "data/fungi.json";
		static constexpr const char* creatures_path = "data/creatures.json";

		std::unordered_map<std::string, nlohmann::json> LUT;
	public:
		static EntityFactory& instance()
		{
			static EntityFactory ef;
			return ef;
		}
		void init();
		void read_definitions(const char* path);
		void add_entities(nlohmann::json& json);
		void log_prototypes() const;

	public:
		entt::entity create_entity(entt::registry& registry, const std::string& name, Cell* cell = nullptr);
};
