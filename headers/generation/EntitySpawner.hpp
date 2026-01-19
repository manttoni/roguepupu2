#pragma once

#include "nlohmann/json.hpp"
#include "external/entt/fwd.hpp"

namespace EntitySpawner
{
	void spawn_entities(entt::registry& registry, nlohmann::json filter, const size_t cave_idx);
}
