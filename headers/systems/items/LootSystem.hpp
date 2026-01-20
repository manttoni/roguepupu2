#pragma once

#include <string>
#include "external/entt/fwd.hpp"

namespace LootSystem
{
	/* Get rolled loot as vector
	 * */
	std::vector<entt::entity> get_loot(entt::registry& registry, const std::string& table_id);
	std::vector<entt::entity> get_loot(entt::registry& registry, const std::vector<std::string>& table_ids);
	/* Give rolled loot directly to entity
	 * */
	void give_loot(entt::registry& registry, const entt::entity entity, const std::string& table_id);
	void give_loot(entt::registry& registry, const entt::entity entity, const std::vector<std::string>& table_ids);
};
