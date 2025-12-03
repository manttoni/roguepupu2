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
		nlohmann::json definitions;
		entt::registry prototypes;
		std::unordered_map<EntityType, entt::entity> LUT;
	public:
		static EntityFactory& instance()
		{
			static EntityFactory ef;
			return ef;
		}
		void init();
		void read_definitions();
		void parse_prototypes();
		void log_prototypes() const;

	public:
		// Create from nothing
		entt::entity create_entity(const EntityType type, Cell& cell);
		//entt::entity create_entity(const EntityType type, const entt::entity entity);
};
