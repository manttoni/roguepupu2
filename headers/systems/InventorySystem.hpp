#pragma once

#include <string>
#include "entt.hpp"

namespace InventorySystem
{
	bool has_item(const entt::registry& registry, const entt::entity holder, const entt::entity item);
	void remove_item(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void add_item(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void take_item(entt::registry& registry, const entt::entity taker, const entt::entity owner, const entt::entity item);
	void drop_item(entt::registry& registry, const entt::entity owner, const entt::entity item);
	double get_inventory_weight(const entt::registry& registry, const entt::entity entity);
};
