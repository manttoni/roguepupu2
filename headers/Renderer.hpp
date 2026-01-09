#pragma once

#include "Cave.hpp"
#include "GameLogger.hpp"
#include "entt.hpp"
#include "ColorPair.hpp"
#include "Liquid.hpp"

class Renderer
{
	private:
		ColorPair ghost_color_pair;
		struct Visual
		{
			ColorPair color_pair;
			wchar_t glyph;
		};
		Visual get_ghost_visual(const Cell& cell) const;
		Visual get_visual(const Cell& cell) const;
		entt::registry& registry;
		size_t render_frame;
		void render_cave();
		void print_log();
		void show_status();
		void show_debug();
		void draw_bar(const Color& color, const double percentage, const size_t y, const size_t x);
	public:
		void render();
		Renderer(entt::registry& registry);
		void render_cell(Cell& cell);
};
