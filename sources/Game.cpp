#include <ncurses.h>
#include <memory>
#include <panel.h>
#include "Game.hpp"
#include "CaveGenerator.hpp"
#include "UI.hpp"
#include "Utils.hpp"
#include "EntityFactory.hpp"

Game::Game() :
	cavegen(CaveGenerator()),
	current_cave(cavegen.get_cave(1))
{
	const size_t start_idx = current_cave.get_source_idx();
	auto& spawn_cell = current_cave.get_cells()[start_idx];

	auto player_ptr = EntityFactory::instance().get_creature("Rabdin");
	auto& player_ref = *player_ptr;

	spawn_cell.add_entity(std::move(player_ptr));
	player_ref.set_cell(&spawn_cell);

	player = &player_ref;

	PANEL* game = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
	UI::instance().add_panel(UI::Panel::GAME, game);

	// log stuff
	Log::log("Game created.");
	assert(player->get_name() == "Rabdin");
}

void Game::start()
{
	int key = 0;
	while (key != KEY_ESCAPE)
	{
		current_cave.draw(*player);
		key = UI::instance().input();

		if (key == KEY_MOUSE)
		{
			MEVENT event;
			if (getmouse(&event) == OK)
			{
				if (event.bstate & BUTTON1_CLICKED)
					move_with_mouse(event);
				else if (event.bstate & BUTTON2_CLICKED)
					action_menu(event);
			}
		}

		if (key == KEY_F(1))
			UI::instance().toggle_show_debug();
	}
}

// left click movement
// if there is an enemy, will attack that enemy also
double Game::move_with_mouse(const MEVENT& event)
{
	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	WINDOW* window = panel_window(panel);
	int window_height, window_width, window_starty, window_startx;
	getmaxyx(window, window_height, window_width);
	getbegyx(window, window_starty, window_startx);

	int mouse_y = event.y - window_starty;
	int mouse_x = event.x - window_startx;

	int window_center_y = window_height / 2;
	int window_center_x = window_width / 2;

	int offset_y = mouse_y - window_center_y;
	int offset_x = mouse_x - window_center_x;

	const size_t player_idx = player->get_idx();
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

		// check for input during movement
		// if input == ERR, there was no input
		// else stop moving
		if (UI::instance().input(100) != ERR)
			break;
	}
	return movement;
}

// aka right click menu
// opens a temporary menu with clickable buttons
// shows all possible actions for the cell clicked
// clickin an option or outside of menu closes it
void Game::action_menu(const MEVENT& event)
{
	(void) event;
}

void start_game()
{
	UI::instance().set_mode(UI::Mode::GAME);
	Game game;
	game.start();
	UI::instance().set_mode(UI::Mode::MAIN);
}
