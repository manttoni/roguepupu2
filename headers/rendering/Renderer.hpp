#pragma once

#include "ui/Theme.hpp"
#include "ncurses/Color.hpp"
#include "domain/Position.hpp"
#include "external/entt/fwd.hpp"
#include "ncurses/Panel.hpp"
#include "utils/Vec2.hpp"

class Renderer
{
	private:
		inline static const UI::Theme theme = UI::load_theme();
		size_t frame = 0;
		Vec2<int> center_offset;
		Ncurses::Panel panel;

		struct Pixel
		{
			Ncurses::Color color{1};
			char glyph = '?';
		};

		std::vector<Domain::Position> get_rendered_positions(const entt::registry& registry, const Domain::Position& center) const;
		Pixel get_pixel(const entt::registry& registry, const Domain::Position& position) const;
		void render_pixel(const Pixel& pixel);

	public:
		void render(const entt::registry& registry, Domain::Position center = Domain::Position::invalid());
};
