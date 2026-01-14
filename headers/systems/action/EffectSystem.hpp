#pragma once

#include "entt.hpp"
#include "Event.hpp"

namespace EffectSystem
{
	void resolve_effect(entt::registry& registry, const Effect& effect, const Target& target);
};
