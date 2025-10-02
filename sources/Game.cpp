#include <ncurses.h>
#include <memory>
#include <panel.h>
#include "Game.hpp"
#include "CaveGenerator.hpp"
#include "UI.hpp"
#include "Utils.hpp"
#include "EntityFactory.hpp"

Game::Game() : level(1)
{

	init_cavegen();
	init_player();
	init_panels();
}

void Game::init_player()
{
	auto& cave = cavegen.get_cave(1);
	assert(cave.get_size() > 0);
	const size_t start_idx = cave.get_source_idx();
	assert(start_idx < cave.get_size());
	auto& spawn_cell = cave.get_cells()[start_idx];
	auto player = EntityFactory::instance().get_creature("Rabdin");
	spawn_cell.add_entity(std::move(player));

	// debug info
	auto& debug = UI::instance().get_menu("debug");
	debug.set_value("player_y", spawn_cell.get_idx() / cave.get_width());
	debug.set_value("player_x", spawn_cell.get_idx() % cave.get_width());
}

void Game::init_cavegen()
{
	cavegen = CaveGenerator();
	Log::log("Game cavegen initialized");
}

void Game::init_panels()
{
	/* MAIN */
	PANEL* game = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
	UI::instance().add_panel(UI::Panel::GAME, game);

	/* LOG */
	//PANEL* log = new_panel(newwin(Screen::height(), Screen::width() / 4, 0, 0));
	//UI::instance().add_panel(UI::Panel::LOG, log);
	Log::log("Game panels initialized");
}

// move this away from the game logic
void Game::draw_cave(Cave& cave)
{
	Log::log("Drawing cave");
	werase(panel_window(UI::instance().get_panel(UI::Panel::GAME)));

	auto& player = get_player();
	assert(player.get_name() == "Rabdin");
	assert(player.get_vision_range() == 10);

	cave.reset_lights();

	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	assert(panel != nullptr);

	WINDOW* window = panel_window(panel);
	assert(window != nullptr);
	werase(window);

	// UI will print to this panel
	UI::instance().set_current_panel(panel);

	// window size
	int height, width;
	getmaxyx(window, height, width);

	// player point in cave cells
	size_t player_idx = player.get_idx();
	size_t y_player = player_idx / cave.get_width();
	size_t x_player = player_idx % cave.get_width();
	assert(y_player < cave.get_height());
	assert(x_player < cave.get_width());

	// center point of screen
	size_t y_center = height / 2;
	size_t x_center = width / 2;

	// get cells in radius
	auto cells_in_vision_range = cave.get_nearby_ids(player_idx, player.get_vision_range());
	cells_in_vision_range.push_back(player_idx);

	for (const auto& cell_idx : cells_in_vision_range)
	{
		// the cell being drawn
		size_t y_cell = cell_idx / cave.get_width();
		size_t x_cell = cell_idx % cave.get_width();

		// point on screen where to draw
		int y = y_center + y_cell - y_player;
		int x = x_center + x_cell - x_player;
		if (y < 0 || y >= height || x < 0 || x >= width)
			continue;
		if (!cave.has_vision(player_idx, cell_idx))
			continue;

		//Log::log("Drawing cell: " + std::to_string(y_cell) + ", " + std::to_string(x_cell));
		const auto& cell = cave.get_cells()[cell_idx];
		const auto& color_pair = cell.get_color_pair();
		wchar_t symbol = cell.get_symbol();
		UI::instance().enable_color_pair(color_pair);
		UI::instance().print_wide(y, x, symbol);
	}
	UI::instance().update();
	Log::log("Cave drawn");
}

Creature& Game::get_player()
{
	const auto& cave = cavegen.get_cave(level);
	for (const auto& cell : cave.get_cells())
	{
		for (const auto& entity : cell.get_entities())
			if (entity->get_name() == "Rabdin")
				return static_cast<Creature&>(*entity);
	}
	throw std::runtime_error("Player is lost :(");
}

void Game::start()
{
	int key = 0;
	Creature& player = get_player();
	while (key != KEY_ESCAPE)
	{
		Cave& cave = cavegen.get_cave(level);
		draw_cave(cave);
		key = UI::instance().input();
		switch (key)
		{
			/*case KEY_ESCAPE: {
				std::string select = UI::instance().ask("Quit?", {"Yes", "No"});
				if (select == "Yes")
					break;
				key = 0;
				continue;
			}*/
			case KEY_DOWN:
				player.move(Direction::DOWN);
				break;
			case KEY_UP:
				player.move(Direction::UP);
				break;
			case KEY_LEFT:
				player.move(Direction::LEFT);
				break;
			case KEY_RIGHT:
				player.move(Direction::RIGHT);
				break;
			case KEY_F(1):
				UI::instance().toggle_show_debug();
				break;
			case KEY_MOUSE:
			{
				MEVENT event;
				if (getmouse(&event) == OK)
				{
					if (event.bstate & BUTTON1_CLICKED)
					{
						// get window size and starting point on screen
						WINDOW* game_window = panel_window(UI::instance().get_panel(UI::Panel::GAME));
						int window_height, window_width, window_starty, window_startx;
						getmaxyx(game_window, window_height, window_width);
						getbegyx(game_window, window_starty, window_startx);

						// click coordinate relative to game window
						int mouse_y = event.y - window_starty;
						int mouse_x = event.x - window_startx;

						// player/center draw coordinate
						int window_center_y = window_height / 2;
						int window_center_x = window_width / 2;

						// distances of click from center
						int offset_y = mouse_y - window_center_y;
						int offset_x = mouse_x - window_center_x;

						// player coordinate of cell in cave
						const size_t player_idx = player.get_idx();
						int player_y = player_idx / cave.get_width();
						int player_x = player_idx % cave.get_width();

						// destination coordinate of cell in cave
						int dest_y = player_y + offset_y;
						int dest_x = player_x + offset_x;
						const size_t dest_idx = dest_y * cave.get_width() + dest_x;

						// find path using A* and assert it is not empty
						auto path = cave.find_path(player_idx, dest_idx);

						// if there is no path, dont do anything
						if (path.empty())
							break;

						std::reverse(path.begin(), path.end());

						// move player through all the cells in the path
						auto& cells = cave.get_cells();
						for (auto& cell_idx : path)
						{
							player.move(cells[cell_idx]);
							draw_cave(cave);
							usleep(100000);
						}
					}
				}
			}
		}
	}
}

void start_game()
{
	UI::instance().set_mode(UI::Mode::GAME);
	Game game;
	game.start();
	UI::instance().set_mode(UI::Mode::MAIN);
}
