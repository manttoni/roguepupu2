#pragma once

#include "external/entt/fwd.hpp"

struct Actor;
struct Effect;
struct Target;
namespace EffectSystem
{
	void resolve_effect(entt::registry& registry, const Actor& actor, const Effect& effect, const Target& target);
};
