#include <string>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include "systems/RenderingSystem.hpp"
#include "entt.hpp"
#include "systems/PositionSystem.hpp"
#include "ECS.hpp"
#include "Components.hpp"
#include "LiquidMixture.hpp"
#include "Color.hpp"
#include "ColorPair.hpp"
#include "Cell.hpp"


namespace RenderingSystem
{
	Visual get_visual(const entt::registry& registry, const Position& position, const bool is_visible)
	{
		Visual visual;
		const auto& cell = PositionSystem::get_cell(registry, position);
		std::vector<entt::entity> visible_entities;
		for (const auto entity : ECS::get_entities(registry, position))
		{
			if (registry.any_of<Hidden, Invisible>(entity) && entity != ECS::get_player(registry))
				continue;
			if (!is_visible && registry.get<Category>(entity).category == "creatures")
				continue;
			visible_entities.push_back(entity);
		}

		const auto& lm = cell.get_liquid_mixture();
		const auto& lm_volume = lm.get_volume();

		// Background color always from cell/liquid
		Color fg, bg;
		if (!is_visible)
			bg = Color::black();
		else if (lm_volume > TRESHOLD_LIQUID_BGCOLOR)
			bg = lm.get_bgcolor();
		else
			bg = cell.get_bgcolor();

		if (visible_entities.empty())
		{
			if (lm_volume > TRESHOLD_LIQUID_DEEP)
				visual.glyph = Unicode::LiquidDeep;
			else if (lm_volume > TRESHOLD_LIQUID_MEDIUM)
				visual.glyph = Unicode::LiquidMedium;
			else if (lm_volume > TRESHOLD_LIQUID_SHALLOW)
				visual.glyph = Unicode::LiquidShallow;
			else if (is_visible || cell.get_type() != Cell::Type::Floor)
				visual.glyph = cell.get_glyph();

			if (lm_volume > TRESHOLD_LIQUID_FGCOLOR)
				fg = lm.get_fgcolor();
			else if (is_visible)
				fg = cell.get_fgcolor();
			else if (cell.get_type() != Cell::Type::Floor)
				fg = Color::grey();
		}
		else
		{
			const size_t render_frame = registry.ctx().get<RenderData>().render_frame;
			const auto visible_entity = visible_entities[render_frame % visible_entities.size()];
			visual.glyph = ECS::get_glyph(registry, visible_entity);
			fg = ECS::get_color(registry, visible_entity);
		}
		for (const auto& [light_color, light_stacks] : cell.get_lights())
		{
			fg += light_color * static_cast<int>(light_stacks);
			bg += light_color * static_cast<int>(light_stacks);
		}
		visual.color_pair = ColorPair(fg, bg);
		return visual;
	}

	void render_cell(const entt::registry& registry, const Position& position, const bool is_visible)
	{
		const Visual visual = get_visual(registry, position, is_visible);
		Vec2 middle = Screen::middle();
		Vec2 cell = PositionSystem::get_coorinates(position);
		Vec2 player = PositionSystem::get_coordinates(registry.get<Position>(ECS::get_player));
		const size_t y = middle.y + cell.y - player.y;
		const size_t x = middle.x + cell.x - player.x;

		UI::instance().enable_color_pair(visual.color_pair);
		UI::instance().print_wide(y, x, visual.glyph);
	}

	void render_active_cave(const entt::registry& registry)
	{
		const auto& cave = ECS::get_active_cave(registry);
		const size_t cave_idx = cave.get_idx();

		auto& render_data = registry.ctx().get<RenderData>();
		render_data.visible_cells = VisionSystem::get_visible_cells(registry, ECS::get_player(registry));
		for (const auto visible_cell : visible_cells)
			render_data.seen_cells[cave_idx].insert(visible_cell);

		for (size_t i = 0; i < cave.get_area() - 1; ++i)
		{
			const Position pos = {.cell_idx = i, .cave_idx = cave_idx};
			auto visible_it = std::find(
					render_data.visible_cells.begin(),
					render_data.visible_cells.end(),
					i);
			if (visible_it != render_data.visible_cells.end())
			{
				render_cell(registry, pos, true);
				continue;
			}

			auto seen_it = std::find(
					render_data.seen_cells[cave_idx].begin(),
					render_data.seen_cells[cave_idx].end(),
					i);
			if (seen_it != render_data.seen_cells[cave_idx].end())
				render_cell(registry, pos, false);
		}
	}

	void print_log(const entt::registry& registry)
	{
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
		const auto& resources = registry.get<Resources>(player);

		const double hp_per = static_cast<double>(resources.health) / static_cast<double>(ECS::get_health_max(registry, player));
		draw_bar(Color(400,0,0), std::max(0.0, hp_per), 1, bar_len);

		const double ft_per = static_cast<double>(resources.fatigue) / static_cast<double>(ECS::get_fatigue_max(registry, player));
		draw_bar(Color(0,400,0), std::max(0.0, ft_per), 3, bar_len);

		const double mp_per = static_cast<double>(resources.mana) / static_cast<double>(ECS::get_mana_max(registry, player));
		draw_bar(Color(0,0,600), std::max(0.0, mp_per), 5, bar_len);
	}

	void show_debug(const entt::registry& registry)
	{
		if (registry.ctx().get<Dev>().show_debug == false)
			return;
		UI::instance().set_current_panel(UI::instance().get_panel(UI::Panel::Game));
		const std::vector<std::string> debug =
		{
			"Colors: " + std::to_string(UI::instance().get_initialized_colors().size()),
			"ColorPairs: " + std::to_string(UI::instance().get_initialized_color_pairs().size()),
			"Turn Number: " + std::to_string(registry.ctx().get<GameState>().turn_number),
			"Liquids volume: " + std::to_string(EnvironmentSystem::get_liquids_volume(ECS::get_active_cave(registry)))
		};
		for (size_t i = 0; i < debug.size(); ++i)
			UI::instance().print(i, 0, debug[i]);
	}

	void render(const entt::registry& registry)
	{
		render_active_cave(registry);
		print_log(registry);
		show_player_status(registry);
		show_debug(registry);
		UI::instance().update();
	}
};
