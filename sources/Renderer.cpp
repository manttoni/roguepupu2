#include <ncurses.h>
#include <panel.h>
#include <string>
#include "systems/VisionSystem.hpp"
#include "entt.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "ColorPair.hpp"
#include "ECS.hpp"
#include "Components.hpp"
#include "Unicode.hpp"
#include "Renderer.hpp"
#include "DevTools.hpp"

Renderer::Renderer(const entt::registry& registry) : registry(registry), render_frame(0)
{
	ghost = ColorPair(Color(123,123,123), Color{});
}

Renderer::Visual Renderer::get_ghost_visual(const Cell& cell) const
{
	if (cell.get_type() == Cell::Type::Rock)
		return {ghost, Unicode::FullBlock};

	for (const auto entity : cell.get_entities())
	{
		if (registry.all_of<Landmark>(entity))
			return {ghost, ECS::get_glyph(registry, entity)};
	}
	Log::error("get_ghost_visual fail");
}

Renderer::Visual Renderer::get_visual(const Cell& cell) const
{
	std::vector<entt::entity> entities;
	for (const auto entity : cell.get_entities())
	{
		if (!registry.any_of<Hidden>(entity) || entity == ECS::get_player(registry))
			entities.push_back(entity);
	}
	Color bg = cell.get_bg();
	Color fg;
	wchar_t glyph = L'?';
	if (entities.empty())
	{
		fg = cell.get_fg();
		glyph = cell.get_glyph();
	}
	else
	{
		const auto visual_entity = entities[render_frame % entities.size()];
		if (registry.any_of<Hidden>(visual_entity)) // If the entity is hidden, it must be the player
			fg = Color(234,234,234);
		else
			fg = ECS::get_color(registry, visual_entity);
		glyph = ECS::get_glyph(registry, visual_entity);
	}

	for (const auto& [light_color, light_stacks] : cell.get_lights())
	{
		fg += light_color * static_cast<int>(light_stacks);
		bg += light_color * static_cast<int>(light_stacks);
	}

	return { ColorPair(fg, bg), glyph };
}

void Renderer::render_cell(Cell& cell)
{
	Cave* cave = cell.get_cave();
	const auto& player = ECS::get_player(registry);
	const auto& player_position = registry.get<Position>(player);
	const size_t player_idx = player_position.cell->get_idx();

	PANEL* panel = UI::instance().get_panel(UI::Panel::Game);
	WINDOW* window = panel_window(panel);
	UI::instance().set_current_panel(panel);

	//werase(window);

	int window_height, window_width;
	getmaxyx(window, window_height, window_width);

	const auto width = cave->get_width();
	size_t y_player = player_idx / width;
	size_t x_player = player_idx % width;

	size_t y_center = window_height / 2;
	size_t x_center = window_width / 2;

	const auto cell_idx = cell.get_idx();
	size_t y_cell = cell_idx / width;
	size_t x_cell = cell_idx % width;

	int y = y_center + y_cell - y_player;
	int x = x_center + x_cell - x_player;
	if (y < 0 || y >= window_height || x < 0 || x >= window_width)
		return;

	Visual visual;
	if (registry.ctx().get<Dev>().god_mode == false &&
			!VisionSystem::has_vision(registry, player, cell))
	{
		if (cell.is_seen() && cell.has_landmark())
			visual = get_ghost_visual(cell);
		else return;
	}
	else visual = get_visual(cell);

	UI::instance().enable_color_pair(visual.color_pair);
	UI::instance().print_wide(y, x, visual.glyph);
	UI::instance().update();
}

void Renderer::render_cave()
{
	Cave& cave = *ECS::get_active_cave(registry);
	const auto& player = ECS::get_player(registry);
	const auto& player_position = registry.get<Position>(player);
	const size_t player_idx = player_position.cell->get_idx();

	PANEL* panel = UI::instance().get_panel(UI::Panel::Game);
	WINDOW* window = panel_window(panel);
	UI::instance().set_current_panel(panel, true);

	werase(window);
	//UI::instance().reset_colors();

	int window_height, window_width;
	getmaxyx(window, window_height, window_width);

	const size_t width = cave.get_width();
	size_t y_player = player_idx / width;
	size_t x_player = player_idx % width;

	size_t y_center = window_height / 2;
	size_t x_center = window_width / 2;

	for (auto& cell : cave.get_cells())
	{
		const size_t cell_idx = cell.get_idx();
		size_t y_cell = cell_idx / width;
		size_t x_cell = cell_idx % width;

		int y = y_center + y_cell - y_player;
		int x = x_center + x_cell - x_player;
		if (y < 0 || y >= window_height || x < 0 || x >= window_width)
			continue;

		Visual visual;
		if (registry.ctx().get<Dev>().god_mode == false &&
				!VisionSystem::has_vision(registry, player, cell))
		{
			if (cell.is_seen() && cell.has_landmark())
				visual = get_ghost_visual(cell);
			else continue;
		}
		else visual = get_visual(cell);

		UI::instance().enable_color_pair(visual.color_pair);
		UI::instance().print_wide(y, x, visual.glyph);
		cell.set_seen(true);
	}
}

void Renderer::print_log()
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

void Renderer::draw_bar(const Color& color, const double percentage, const size_t y, const size_t width)
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

void Renderer::show_status()
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

void Renderer::show_debug()
{
	UI::instance().set_current_panel(UI::instance().get_panel(UI::Panel::Game));
	const std::vector<std::string> debug =
	{
		"Colors: " + std::to_string(UI::instance().get_initialized_colors().size()),
		"ColorPairs: " + std::to_string(UI::instance().get_initialized_color_pairs().size())
	};
	for (size_t i = 0; i < debug.size(); ++i)
		UI::instance().print(i, 0, debug[i]);
}

void Renderer::render()
{
	render_cave();
	print_log();
	show_status();
	if (registry.ctx().get<Dev>().show_debug == true)
		show_debug();
	UI::instance().update();
	render_frame++;
}
