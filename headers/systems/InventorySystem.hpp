#pragma once

#include <string>
#include "entt.hpp"

namespace InventorySystem
{
	bool inventory_key_pressed(const int key);
	bool has_item(const entt::registry& registry, const entt::entity holder, const entt::entity item);
	void remove_item(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void add_item(entt::registry& registry, const entt::entity entity, const entt::entity item);
	void loot_item(entt::registry& registry, const entt::entity looter, const entt::entity owner, const entt::entity item);
	void open_inventory(entt::registry& registry, const entt::entity entity);
};
