#pragma once

#include "entt.hpp"  // for entity, registry, size_t
class Color;

namespace VisualEffectSystem
{
	void flash_entity(entt::registry& registry, const entt::entity entity, const Color& fgcolor, const size_t ms = 100);
	void damage_flash(entt::registry& registry, const entt::entity entity);
};
