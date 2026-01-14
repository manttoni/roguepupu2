#pragma once

#include "entt.hpp"
#include "ColorPair.hpp"

#define TRESHOLD_LIQUID_BGCOLOR 0.0
#define TRESHOLD_LIQUID_FGCOLOR 0.0

namespace RenderingSystem
{
	struct Visual
	{
		wchar_t glyph = L' ';
		ColorPair color_pair{};
	};
	void render(const entt::registry& registry);
};
