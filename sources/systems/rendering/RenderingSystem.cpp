#include <curses.h>
#include <panel.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

#include "UI/UI.hpp"
#include "components/Components.hpp"
#include "domain/Cell.hpp"
#include "domain/Color.hpp"
#include "domain/ColorPair.hpp"
#include "domain/LiquidMixture.hpp"
#include "external/entt/entt.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/rendering/RenderData.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Unicode.hpp"
#include "utils/Screen.hpp"
#include "infrastructure/GameLogger.hpp"
#include "utils/Math.hpp"
#include "infrastructure/DevTools.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "domain/Cave.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"
#include "infrastructure/GameState.hpp"
#include "utils/Error.hpp"
#include "utils/Vec2.hpp"

namespace RenderingSystem
{
	wchar_t get_glyph(const entt::registry& registry, const Position& position)
	{
		const auto& cell = ECS::get_cell(registry, position);

		const auto& lm = cell.get_liquid_mixture();
		if (lm.get_volume() > 100.0)
			return Unicode::LiquidDeep;
		if (lm.get_volume() > 10.0)
			return Unicode::LiquidMedium;
		if (lm.get_volume() > 0)
			return Unicode::LiquidShallow;

		return cell.get_glyph();
	}

	Color get_fgcolor(const entt::registry& registry, const Position& position)
	{
		const auto& cell = ECS::get_cell(registry, position);

		const auto& lm = ECS::get_cell(registry, position).get_liquid_mixture();
		if (lm.get_volume() > 0)
			return lm.get_color();

		const auto type = cell.get_type();

		switch (type)
		{
			case Cell::Type::Rock:
				// Set color gradient by density in some range
				return Color(25, 30, 20) * static_cast<int>(std::round(std::min(
								static_cast<double>(CELL_DENSITY_MAX),
								cell.get_density() / 2.0 + 2.0
								)));
			case Cell::Type::Floor:
				return Color(25, 30, 20) * 2;
			case Cell::Type::Source:
			case Cell::Type::Sink:
				return Color(80, 160, 80);
			default:
				Error::fatal("Cell without type");
		}
	}

	Color get_bgcolor(const entt::registry& registry, const Position& position)
	{
		const auto& cell = ECS::get_cell(registry, position);

		const auto& lm = ECS::get_cell(registry, position).get_liquid_mixture();
		if (lm.get_volume() > 1)
			return lm.get_color() / 5;

		const auto type = cell.get_type();

		switch (type)
		{
			case Cell::Type::Rock:
			case Cell::Type::Floor:
			case Cell::Type::Source:
			case Cell::Type::Sink:
				return Color(25, 30, 20);
			default:
				Error::fatal("Cell without type");
		}
	}

	Visual get_visual(const entt::registry& registry, const Position& position)
	{
		const size_t render_frame = registry.ctx().get<RenderData>().render_frame;
		std::vector<entt::entity> visible_entities =
			VisionSystem::get_visible_entities_in_position(registry, ECS::get_player(registry), position);
		const auto visible_entity =
			!visible_entities.empty() ?
			visible_entities[render_frame % visible_entities.size()] :
			entt::null;

		Visual visual;
		visual.attr = ECS::get_cell(registry, position).get_attr();
		if (visible_entity != entt::null)
		{
			visual.glyph = ECS::get_glyph(registry, visible_entity);
			visual.fg = ECS::get_fgcolor(registry, visible_entity);
			visual.bg = get_bgcolor(registry, position);
			if (registry.all_of<NcursesAttr>(visible_entity))
				visual.attr |= registry.get<NcursesAttr>(visible_entity).attr;
		}
		else
		{
			visual.glyph = get_glyph(registry, position);
			visual.fg = get_fgcolor(registry, position);
			visual.bg = get_bgcolor(registry, position);
		}

		for (const auto& [light_color, light_stacks] : ECS::get_cell(registry, position).get_lights())
		{
			visual.fg += light_color * static_cast<int>(light_stacks);
			visual.bg += light_color * static_cast<int>(light_stacks);
		}
		visual.color_pair = ColorPair(visual.fg, visual.bg);
		return visual;
	}

	void render_cell(const entt::registry& registry, const Position& position)
	{
		UI::instance().set_current_panel(UI::Panel::Game, true);
		const auto& cave = ECS::get_cave(registry, position);
		const Visual visual = get_visual(registry, position);
		const Vec2 middle = Screen::middle();
		const Vec2 cell(position.cell_idx, cave.get_size());
		const Vec2 player(registry.get<Position>(ECS::get_player(registry)).cell_idx, cave.get_size());
		const int y = middle.y + cell.y - player.y;
		const int x = middle.x + cell.x - player.x;
		if (y > Screen::height() || x > Screen::width() ||
				y < 0 || x < 0)
			return;

		UI::instance().enable_color_pair(visual.color_pair);
		UI::instance().enable_attr(visual.attr);
		UI::instance().print_wide(y, x, visual.glyph);
		UI::instance().disable_attr(visual.attr);
		UI::instance().disable_color_pair(visual.color_pair);
	}

	void render_active_cave(entt::registry& registry)
	{
		UI::instance().set_current_panel(UI::Panel::Game, true);
		werase(panel_window(UI::instance().get_current_panel()));

		// print seen cells with black and grey, probably only rock and (floor)

		const auto visible_positions = VisionSystem::get_visible_positions(registry, ECS::get_player(registry));
		for (const auto position : visible_positions)
			render_cell(registry, position);
	}

	void print_log(const entt::registry& registry)
	{
		if (registry.ctx().get<RenderData>().print_log == false)
			return;
		const auto logger = registry.ctx().get<GameLogger>();
		const auto& messages = logger.last(20);
		ColorPair log_pair = ColorPair(Color(500, 500, 500), Color{});
		const size_t n = messages.size();
		const size_t height = Screen::height();
		PANEL* panel = UI::instance().get_panel(UI::Panel::Game);
		WINDOW* window = panel_window(panel);
		UI::instance().enable_color_pair(log_pair);
		for (size_t i = 0; i < n; ++i)
		{
			wmove(window, height - n + i, 0);
			UI::instance().print(messages[i]);
		}
		UI::instance().disable_color_pair(log_pair);
	}

	void draw_bar(const Color& color, const double percentage, const size_t y, const size_t width)
	{
		const size_t full_blocks = width * percentage;
		const double per_block = 1.0 / static_cast<double>(width);
		const double remaining = percentage - full_blocks * per_block;
		std::wstring bar(full_blocks, Unicode::FullBlock);
		if (remaining > 0)
			bar += Unicode::LeftBlocks[static_cast<size_t>(Math::map(remaining, 0, per_block, 0, 8))];

		PANEL* status_panel = UI::instance().get_panel(UI::Panel::Status);

		UI::instance().set_current_panel(status_panel, true);
		UI::instance().enable_color_pair(ColorPair(color, Color{}));
		UI::instance().print_wstr(y, 1, bar);

		UI::instance().disable_color_pair(ColorPair(color, Color{}));
	}

	void show_player_status(const entt::registry& registry)
	{
		const size_t bar_len = 25;
		PANEL* status_panel = UI::instance().get_panel(UI::Panel::Status);
		WINDOW* status_window = panel_window(status_panel);
		werase(status_window);
		box(status_window, 0, 0);
		mvwhline(status_window, 2, 1, ACS_HLINE, bar_len);
		mvwhline(status_window, 4, 1, ACS_HLINE, bar_len);

		const auto player = ECS::get_player(registry);

		const double hp_per = static_cast<double>(registry.get<Health>(player).current) / static_cast<double>(StateSystem::get_max_health(registry, player));
		draw_bar(Color(400,0,0), std::max(0.0, hp_per), 1, bar_len);

		const double ft_per = static_cast<double>(registry.get<Stamina>(player).current) / static_cast<double>(StateSystem::get_max_stamina(registry, player));
		draw_bar(Color(0,400,0), std::max(0.0, ft_per), 3, bar_len);

		const double mp_per = static_cast<double>(registry.get<Mana>(player).current) / static_cast<double>(StateSystem::get_max_mana(registry, player));
		draw_bar(Color(0,0,600), std::max(0.0, mp_per), 5, bar_len);
	}

	void show_debug(const entt::registry& registry)
	{
		if (registry.ctx().get<Dev>().show_debug == false)
			return;
		UI::instance().set_current_panel(UI::Panel::Game);
		const std::vector<std::string> debug =
		{
			"Colors: " + std::to_string(UI::instance().get_initialized_colors().size()),
			"ColorPairs: " + std::to_string(UI::instance().get_initialized_color_pairs().size()),
			"Turn Number: " + std::to_string(registry.ctx().get<GameState>().turn_number),
			"Liquids volume: " + std::to_string(LiquidSystem::get_liquids_volume(registry, ECS::get_active_cave(registry).get_idx()))
		};
		for (size_t i = 0; i < debug.size(); ++i)
			UI::instance().print(i, 0, debug[i]);
	}

	void render(entt::registry& registry)
	{
		render_active_cave(registry);
		print_log(registry);
		show_player_status(registry);
		show_debug(registry);
		UI::instance().update();
		registry.ctx().get<RenderData>().render_frame++;
		UI::instance().enable_attr(A_NORMAL);
	}

	void render_generation(const entt::registry& registry, const size_t cave_idx)
	{
		UI::instance().set_current_panel(UI::Panel::Game);
		const auto& cave = ECS::get_cave(registry, cave_idx);
		for (const auto& pos : cave.get_positions())
		{
			const Visual visual = get_visual(registry, pos);
			const Vec2 coords(pos.cell_idx, cave.get_size());
			UI::instance().enable_color_pair(visual.color_pair);
			if (visual.attr != A_NORMAL)
				UI::instance().enable_attr(visual.attr);
			UI::instance().print_wide(coords.y, coords.x, visual.glyph);
			if (visual.attr != A_NORMAL)
				UI::instance().disable_attr(visual.attr);
			UI::instance().disable_color_pair(visual.color_pair);
		}
		UI::instance().update();
	}

};
