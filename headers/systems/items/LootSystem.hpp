#pragma once

#include <string>
#include "external/entt/fwd.hpp"

namespace LootSystem
{
	std::vector<entt::entity> get_loot(entt::registry& registry, const std::string& table_id);
};
