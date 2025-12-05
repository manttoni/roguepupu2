#pragma once

#include <string>
#include "entt.hpp"

namespace InventorySystem
{
	bool inventory_key_pressed(const int key);
	void open_inventory(entt::registry& registry, const entt::entity entity);
};
