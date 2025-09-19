#include <ncurses.h>
#include <panel.h>
#include "Game.hpp"
#include "CaveGenerator.hpp"
#include "UI.hpp"
#include "Utils.hpp"


Game::Game() : level(1), player(Creature("Rabdin", '@', UI::instance().add_color(123, 456, 789)))
{
	init_cavegen();
	init_panels();
}

void Game::init_cavegen()
{
	cavegen = CaveGenerator();
}

void Game::init_panels()
{
	/* MAIN */
	PANEL* main = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
	UI::instance().add_panel(UI::Panel::MAIN, main);

	/* LOG */
	PANEL* log = new_panel(newwin(Screen::height(), Screen::width() / 4, 0, 0));
	UI::instance().add_panel(UI::Panel::LOG, log);
}

void Game::draw_cell(const Cell& cell) const
{
	UI::instance().enable_attr(COLOR_PAIR(cell.get_color_pair_id()));

	UI::instance().print(cell.get_char());

	UI::instance().disable_attr(COLOR_PAIR(cell.get_color_pair_id()));
}

void Game::draw_cave(Cave& cave) const
{
	cave.reset_effects();
	PANEL* panel = UI::instance().get_panel(UI::Panel::MAIN);
	WINDOW* window = panel_window(panel);
	UI::instance().set_current_panel(panel);
	int height, width;
	getmaxyx(window, height, width);
	size_t player_idx = player.get_idx();
	size_t y_player = player_idx / width;
	size_t x_player = player_idx % width;
	size_t y_center = height / 2;
	size_t x_center = width / 2;
	int y_draw = y_center - y_player;
	int x_draw = x_center - x_player;

	wmove(window, 0, 0);
	const auto& cells = cave.get_cells();
	for (size_t i = 0; i < cave.get_size(); ++i)
	{
		if (cave.has_vision(player.get_idx(), i) && y_draw < height - 1 && y_draw >= 0
			&& x_draw < width - 1 && x_draw >= 0)
		{
			wmove(window, y_draw, x_draw);
			draw_cell(cells[i]);
		}

		x_draw++;
		if ((i + 1) % cave.get_width() == 0)
		{
			y_draw++;
			x_draw -= cave.get_width();
		}
	}
	UI::instance().update();
}

void Game::start()
{
	auto& start_cell = cavegen.get_cave(1).get_cells()[cavegen.get_cave(1).get_source()];
	start_cell.add_entity(player);
	player.set_cell(&start_cell);
	int key = 0;
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
