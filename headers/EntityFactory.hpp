#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include "entt.hpp"
#include "Cell.hpp"
#include "entity_enums.hpp"

class EntityFactory
{
	private:
		static constexpr const char* path = "data/entities.json";
		nlohmann::ordered_json definitions;
		entt::registry prototypes;
		std::unordered_map<EntityType, nlohmann::json> LUT;
	public:
		static EntityFactory& instance()
		{
			static EntityFactory ef;
			return ef;
		}
		void init();
		void read_definitions();
		void create_lut();
		void log_prototypes() const;

	public:
		entt::entity create_entity(const EntityType type, Cell& cell);
};
