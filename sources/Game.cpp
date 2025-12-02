#include <ncurses.h>
#include <memory>
#include <panel.h>
#include "Game.hpp"
#include "CaveGenerator.hpp"
#include "UI.hpp"
#include "Utils.hpp"

Game::Game() :
	cavegen(CaveGenerator()),
	current_cave(cavegen.get_cave(1))
{
	const size_t start_idx = current_cave.get_source_idx();
	auto& spawn_cell = current_cave.get_cells()[start_idx];
	current_cave.create_entity("player", spawn_cell);

	PANEL* game = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
	UI::instance().add_panel(UI::Panel::GAME, game);

	// log stuff
	Log::log("Game created.");
}

void Game::start()
{
	int key = 0;
	while (key != KEY_ESCAPE)
	{
		current_cave.draw();
		key = UI::instance().input(500);

		switch (key)
		{
			/*case KEY_LEFT_CLICK:
				move_with_mouse();
				break;*/
			/*case KEY_RIGHT_CLICK:
				action_menu();
				break;*/
			case KEY_ESCAPE:
				if (UI::instance().dialog("Quit to main menu?", {"Yes", "No"}) == "No")
					key = 0;
				break;
		}
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

// aka right click menu
// opens a temporary menu with clickable buttons
// shows all possible actions for the cell clicked
// clickin an option or outside of menu closes it
void Game::action_menu()
{}

void new_game()
{
	UI::instance().set_mode(UI::Mode::GAME);
	Game game;
	game.start();
	game.end();
	UI::instance().set_mode(UI::Mode::MAIN);
}
