#include <utility>
#include <ncurses.h>
#include <memory>
#include <panel.h>
#include "EntityFactory.hpp"
#include "Game.hpp"
#include "World.hpp"
#include "UI.hpp"
#include "Components.hpp"
#include "Utils.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/InventorySystem.hpp"

Game::Game() :
	level(1),
	player(EntityFactory::instance().create_entity(world.get_registry(), "rabdin", &get_cave().get_source()))
{}

void Game::check_change_level()
{
	const auto& pos = get_registry().get<Position>(player);
	const auto current_idx = pos.cell->get_idx();
	const auto ends = get_cave().get_ends();
	if (current_idx != ends.first && current_idx != ends.second)
		return;

	const int d = current_idx == ends.first ? -1 : 1;
	if (level + d < 1)
		return;

	if (UI::instance().dialog("Change level?", {"Yes", "No"}) != "Yes")
		return;

	auto& new_cave = world.get_cave(level + d);
	const size_t new_idx = d > 0 ? new_cave.get_source_idx() : new_cave.get_sink_idx();
	auto& new_cell = new_cave.get_cell(new_idx);

	MovementSystem::move(get_registry(), player, new_cell);
	level += d;
}

void Game::start()
{
	get_cave().draw();

	int key = 0;
	while (key != KEY_ESCAPE)
	{
		key = UI::instance().input(500);
		if (key == KEY_ESCAPE)
		{
			if (UI::instance().dialog("Quit to main menu?", {"Yes", "No"}) == "Yes")
				return;
			key = 0;
		}
		if (MovementSystem::movement_key_pressed(key))
		{
			if (MovementSystem::move(get_registry(), player, key) == 0)
				continue;
			check_change_level();
		}
		if (InventorySystem::inventory_key_pressed(key))
		{
			InventorySystem::open_inventory(get_registry(), player);
			key = 0;
			continue;
		}

		get_cave().draw();
		UI::instance().increase_loop_number();
	}
}

void Game::end()
{
	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	WINDOW* win = panel_window(panel);
	wclear(win);
	UI::instance().update();
	del_panel(panel);
	delwin(win);
}

// Called by main menu
void new_game()
{
	UI::instance().set_mode(UI::Mode::GAME);
	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	UI::instance().set_current_panel(panel, true);

	Game game;
	game.start();
	game.end();
	UI::instance().set_mode(UI::Mode::MAIN);
}
