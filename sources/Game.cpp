#include <ncurses.h>
#include <memory>
#include <panel.h>
#include "Game.hpp"
#include "CaveGenerator.hpp"
#include "UI.hpp"
#include "Components.hpp"
#include "Utils.hpp"
#include "systems/MovementSystem.hpp"

Game::Game() :
	cavegen(CaveGenerator()),
	current_cave(&cavegen.get_cave(1))
{
	const size_t start_idx = current_cave->get_source_idx();
	auto& spawn_cell = current_cave->get_cells()[start_idx];

	auto& registry = current_cave->get_registry();
	entt::entity player = registry.create();
	registry.emplace<Renderable>(player, L'@', Color(123,456,789));
	registry.emplace<Player>(player);
	registry.emplace<Solid>(player);
	registry.emplace<Vision>(player, 10);
	registry.emplace<Inventory>(player);
	registry.emplace<Name>(player, "Rabdin");
	registry.emplace<Position>(player, &spawn_cell);

	PANEL* game = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
	UI::instance().add_panel(UI::Panel::GAME, game);
}

entt::entity Game::get_player()
{
	return *current_cave->get_registry().view<Player>().begin();
}

void Game::change_level(const int d)
{
	int level = current_cave->get_level();
	if (level + d < 1)
		return;

	auto* next_cave = &cavegen.get_cave(level + d);

	auto& current_registry = current_cave->get_registry();
	auto& next_registry = next_cave->get_registry();

	auto current_player = get_player();
	auto next_player = next_registry.create();

	size_t spawn_cell_idx;
	if (d > 0)
		spawn_cell_idx = next_cave->get_source_idx();
	else
		spawn_cell_idx = next_cave->get_sink_idx();
	auto& spawn_cell = next_cave->get_cells()[spawn_cell_idx];

	next_registry.emplace<Renderable>(next_player, L'@', Color(123,456,789));
	next_registry.emplace<Player>(next_player);
	next_registry.emplace<Solid>(next_player);
	next_registry.emplace<Vision>(next_player, 10);
	next_registry.emplace<Inventory>(next_player);
	next_registry.emplace<Name>(next_player, "Rabdin");
	next_registry.emplace<Position>(next_player, &spawn_cell);

	current_registry.destroy(current_player);
	current_cave = next_cave;
}

void Game::check_descend()
{
	const size_t player_idx = current_cave->get_registry().get<Position>(get_player()).cell->get_idx();
	if (player_idx == current_cave->get_sink_idx())
	{
		if (UI::instance().dialog("Go deeper?", {"Yes", "No"}) == "Yes")
			change_level(1);
	}
	else if (current_cave->get_level() > 1 && player_idx == current_cave->get_source_idx())
	{
		if (UI::instance().dialog("Go back?", {"Yes", "No"}) == "Yes")
			change_level(-1);
	}
}

void Game::start()
{
	current_cave->draw();
	int key = 0;
	while (key != KEY_ESCAPE)
	{
		key = UI::instance().input(500);
		if (key == KEY_ESCAPE)
		{
			if (UI::instance().dialog("Quit to main menu?", {"Yes", "No"}) == "No")
				key = 0;
			break;
		}

		if (MovementSystem::movement_keys.find(key) != MovementSystem::movement_keys.end())
		{
			if (MovementSystem::move(current_cave->get_registry(), get_player(), MovementSystem::movement_keys[key]) > 0)
				check_descend();
			else
				continue;
		}

		current_cave->draw();
		UI::instance().increase_loop_number();
	}
}

/*
// left click movement
// if there is an enemy, will attack that enemy also
double Game::move_with_mouse()
{
	Screen::Coord mouse_position = UI::instance().get_mouse_position();
	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	WINDOW* window = panel_window(panel);
	int window_height, window_width, window_starty, window_startx;
	getmaxyx(window, window_height, window_width);
	getbegyx(window, window_starty, window_startx);

	int mouse_y = mouse_position.y - window_starty;
	int mouse_x = mouse_position.x - window_startx;

	int window_center_y = window_height / 2;
	int window_center_x = window_width / 2;

	int offset_y = mouse_y - window_center_y;
	int offset_x = mouse_x - window_center_x;

	const size_t player_idx = get_player()->get_idx();
	int player_y = player_idx / current_cave.get_width();
	int player_x = player_idx % current_cave.get_width();

	int dest_y = player_y + offset_y;
	int dest_x = player_x + offset_x;
	const size_t dest_idx = dest_y * current_cave.get_width() + dest_x;

	auto path = current_cave.find_path(player_idx, dest_idx);
	std::reverse(path.begin(), path.end());
	if (path.empty())
		return 0;

	auto& cells = current_cave.get_cells();
	double movement = 0;
	for (auto& cell_idx : path)
	{
		movement += player->move(cells[cell_idx]);
		current_cave.draw(*player);

		// stop with any input
		if (UI::instance().input(100) != ERR)
			break;
	}
	return movement;
}
*/

void Game::end()
{
	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	WINDOW* win = panel_window(panel);
	wclear(win);
	UI::instance().update();
	del_panel(panel);
	delwin(win);

}

void new_game()
{
	UI::instance().set_mode(UI::Mode::GAME);
	Game game;
	game.start();
	game.end();
	UI::instance().set_mode(UI::Mode::MAIN);
}
