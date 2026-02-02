#pragma once

#include <ncurses.h>
#include "domain/ColorPair.hpp"
#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"
#include "domain/Cave.hpp"

#define TRESHOLD_LIQUID_BGCOLOR 0.0
#define TRESHOLD_LIQUID_FGCOLOR 0.0

namespace RenderingSystem
{
	struct Visual
	{
		wchar_t glyph = L' ';
		Color fg;
		Color bg;
		ColorPair color_pair{};
		chtype attr = A_NORMAL;
	};
	Visual get_visual(const entt::registry& registry, const Position& position);
	void render_cell(const entt::registry& registry, const Position& position);
	void render(entt::registry& registry);
	void render_generation(const entt::registry& registry, const size_t cave_idx);
};
