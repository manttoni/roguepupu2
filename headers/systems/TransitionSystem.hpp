#pragma once

#include "entt.hpp"

namespace TransitionSystem
{
	void transition(entt::registry& registry, const entt::entity entity, const entt::entity portal);
	bool is_portal(const entt::registry& registry, const entt::entity entity);
};
