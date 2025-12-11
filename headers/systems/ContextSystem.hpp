#pragma once

#include "entt.hpp"
class Cell;

namespace ContextSystem
{
	void show_entity_details(entt::registry& registry, const entt::entity entity, const entt::entity owner = entt::null);
	void show_entities_list(entt::registry& registry, const entt::entity owner);
	void show_entities_list(entt::registry& registry, Cell* cell);
};
