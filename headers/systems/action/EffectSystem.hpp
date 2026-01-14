#pragma once

#include "external/entt/fwd.hpp"

struct Effect;
struct Target;

namespace EffectSystem
{
	void resolve_effect(entt::registry& registry, const Effect& effect, const Target& target);
};
