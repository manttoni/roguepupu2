#pragma once

#include "domain/ColorPair.hpp"
#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

#define TRESHOLD_LIQUID_BGCOLOR 0.0
#define TRESHOLD_LIQUID_FGCOLOR 0.0

namespace RenderingSystem
{
	struct Visual
	{
		wchar_t glyph = L' ';
		ColorPair color_pair{};
	};
	void render_cell(const entt::registry& registry, const Position& position, const bool is_seen);
	void render(const entt::registry& registry);
};
