#include "Renderer.hpp"

#include <ncurses.h>
#include <panel.h>
#include <string>
#include "entt.hpp"
#include "Cave.hpp"
#include "Cell.hpp"
#include "ColorPair.hpp"
#include "World.hpp"
#include "ECS.hpp"
#include "Components.hpp"
#include "Unicode.hpp"

void Renderer::draw_cave(Cave& cave)
{
	auto& registry = cave.get_world()->get_registry();
	const auto& player = ECS::get_player(registry);
	const auto& player_position = registry.get<Position>(player);
	const size_t player_idx = player_position.cell->get_idx();

	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	WINDOW* window = panel_window(panel);
	UI::instance().set_current_panel(panel, true);

	werase(window);
	UI::instance().reset_colors();
	//reset_lights();

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

		ColorPair color_pair;

		if (!cave.has_vision(player_idx, cell_idx, registry.get<Vision>(player).range))
		{
			if (cell.is_seen() && cell.blocks_movement()) // "ghost" cell if it was seen before and was solid
				color_pair = ColorPair(Color(123, 123, 123), Color(0, 0, 0));
			else
				continue;
		}
		else
			color_pair = cell.get_color_pair();

		wchar_t glyph = cell.get_glyph();
		UI::instance().enable_color_pair(color_pair);
		UI::instance().print_wide(y, x, glyph);
		cell.set_seen(true);
	}
}

void Renderer::print_log(const GameLogger& logger)
{
	const auto& messages = logger.last(20);
	ColorPair log_pair = ColorPair(Color(500, 500, 500), Color{});
	const size_t n = messages.size();
	const size_t height = Screen::height();
	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
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

	PANEL* status_panel = UI::instance().get_panel(UI::Panel::STATUS);

	UI::instance().set_current_panel(status_panel, true);
	UI::instance().enable_color_pair(ColorPair(color, Color{}));
	UI::instance().print_wstr(y, 1, bar);
	UI::instance().disable_color_pair(ColorPair(color, Color{}));
}

void Renderer::show_status(const entt::registry& registry)
{
	const size_t bar_len = 25;
	PANEL* status_panel = UI::instance().get_panel(UI::Panel::STATUS);
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

void Renderer::render(Cave& cave)
{
	const entt::registry& registry = cave.get_world()->get_registry();
	draw_cave(cave);
	print_log(registry.ctx().get<GameLogger>());
	show_status(registry);
	UI::instance().update();
}
