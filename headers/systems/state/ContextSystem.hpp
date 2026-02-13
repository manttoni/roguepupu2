#pragma once

#include "external/entt/fwd.hpp"

struct Position;
namespace ContextSystem
{
	/* Show details of one entity. It can be on the ground or in someones inventory
	 * */
	void show_entity_details(entt::registry& registry, const entt::entity entity, const entt::entity owner = entt::null);

	/* Player opens inventory. Inventory can be players or other entitys.
	 * Shows list of entities inside. Selecting one will show_entity_details
	 * */
	void open_inventory(entt::registry& registry, const entt::entity entity);

	/* Make a list of entities on the ground. Selecting one will show_entity_details.
	 * Can also show liquid type and level, maybe density?
	 * */
	void examine_cell(entt::registry& registry, const Position& position);

};
