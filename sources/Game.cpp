#include <ncurses.h>                    // for delwin, wclear, WINDOW
#include <panel.h>                      // for PANEL, del_panel, panel_window
#include <string>                       // for basic_string, allocator, oper...
#include <utility>                      // for pair
#include "Cave.hpp"                     // for Cave
#include "Cell.hpp"                     // for Cell
#include "Components.hpp"               // for Position
#include "EntityFactory.hpp"            // for EntityFactory
#include "Game.hpp"                     // for Game, new_game
#include "UI.hpp"                       // for UI, KEY_ESCAPE
#include "World.hpp"                    // for World
#include "entt.hpp"                     // for allocator, vector, size_t
#include "systems/ContextSystem.hpp"
#include "systems/InventorySystem.hpp"  // for inventory_key_pressed, open_i...
#include "systems/MovementSystem.hpp"   // for move, movement_key_pressed
#include "ECS.hpp"

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

void Game::loop()
{
	get_cave().draw();

	int key;
	while ((key = UI::instance().input(500)) != KEY_ESCAPE)
	{
		if (key == KEY_LEFT_CLICK)
		{
			Cell* clicked_cell = UI::instance().get_clicked_cell(get_cave());
			auto entities = clicked_cell->get_entities();
			if (entities.size() == 1)
				ContextSystem::show_entity_details(get_registry(), entities[0]);
			else
				ContextSystem::show_entities_list(get_registry(), clicked_cell);
		}
		if (MovementSystem::move(get_registry(), player, key) > 0)
			check_change_level();
		if (key == 'i')
			ContextSystem::show_entities_list(get_registry(), player);
		if (key == 'c')
			ContextSystem::show_entity_details(get_registry(), player);

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
	PANEL* game_panel = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
	UI::instance().add_panel(UI::Panel::GAME, game_panel);
	UI::instance().set_mode(UI::Mode::GAME);
	PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
	UI::instance().set_current_panel(panel, true);

	Game game;
	game.loop();
	game.end();
	UI::instance().set_mode(UI::Mode::MAIN);
}
