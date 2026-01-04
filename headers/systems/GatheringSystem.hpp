#pragma once

#include "entt.hpp"

namespace GatheringSystem
{
	bool can_gather(const entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable);
	void gather(entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable);
};
