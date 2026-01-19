#pragma once

#include "nlohmann/json.hpp"
#include "external/entt/fwd.hpp"

namespace EntitySpawner
{
	/* Entities that have defined spawn conditions in .json can spawn here
	 * */
	void spawn_entities(entt::registry& registry, const size_t cave_idx, nlohmann::json filter = {});

	/* Existing entities that don't meet their spawn conditions are despawned
	 * */
	void despawn_entities(entt::registry& registry, const size_t cave_idx);
}
