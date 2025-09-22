#include <ncurses.h>
#include <memory>
#include <panel.h>
#include "Game.hpp"
#include "CaveGenerator.hpp"
#include "UI.hpp"
#include "Utils.hpp"


Game::Game() : level(1)
{

	init_cavegen();
	init_player();
	init_panels();
}

void Game::init_player()
{
	const short player_color_id = UI::instance().add_color(123,456,789);
	auto& cave = cavegen.get_cave(1);
	const size_t start_idx = cave.get_source();
	auto& spawn_cell = cave.get_cells()[start_idx];
	auto player = std::make_unique<Creature>(Creature("Rabdin", player_color_id, '@', &spawn_cell));
	spawn_cell.add_entity(std::move(player));
	Log::log("Player initialized");

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
	PANEL* main = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
	UI::instance().add_panel(UI::Panel::MAIN, main);

	/* LOG */
	//PANEL* log = new_panel(newwin(Screen::height(), Screen::width() / 4, 0, 0));
	//UI::instance().add_panel(UI::Panel::LOG, log);
	Log::log("Game panels initialized");
}

void Game::draw_cell(const Cell& cell) const
{
	UI::instance().enable_attr(COLOR_PAIR(cell.get_color_pair_id()));

	UI::instance().print(cell.get_char());

	UI::instance().disable_attr(COLOR_PAIR(cell.get_color_pair_id()));
}

void Game::draw_cave(Cave& cave)
{
	Log::log("Drawing cave");
	auto& player = get_player();
	assert(player.get_name() == "Rabdin");
	assert(player.get_char() == '@');
	assert(player.get_vision_range() == 10);
	assert(player.get_idx() == cave.get_source());
	cave.reset_effects(); // remove all lights and reapply them
	PANEL* panel = UI::instance().get_panel(UI::Panel::MAIN);
	assert(panel != nullptr);
	WINDOW* window = panel_window(panel);
	assert(window != nullptr);
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

		wmove(window, y, x);
		/*
		if (!player.can_see_cell(cell_idx))
		{
			if (player.has_seen_cell(cell_idx))
			{
				UI::instance().print
			}
			continue;
		}*/
		const auto& cell = cave.get_cells()[cell_idx];
		const short cp_id = cell.get_color_pair_id();
		UI::instance().enable_attr(COLOR_PAIR(cp_id));
		UI::instance().print(cell.get_char());
		UI::instance().disable_attr(COLOR_PAIR(cp_id));


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
	throw std::runtime_error("Player is lost");
}

void Game::start()
{
	int key = 0;
	Entity& player = get_player();
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
