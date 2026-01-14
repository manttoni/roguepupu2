#pragma once

#include "external/entt/fwd.hpp"

namespace GatheringSystem
{
	bool has_tool(const entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable);
	bool can_gather(const entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable);
	void gather(entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable);
};
