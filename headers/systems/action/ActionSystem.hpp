#pragma once

#include <cstdint>
#include "external/entt/fwd.hpp"

struct Intent;
namespace ActionSystem
{
	void resolve_intent(entt::registry& registry, const Intent& intent);
	void act_round(entt::registry& registry, const size_t cave_idx);
};
