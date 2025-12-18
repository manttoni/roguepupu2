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
{
	get_registry().ctx().emplace<GameLogger>(log);
}

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

void Game::handle_key(const int key)
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
	else if (MovementSystem::move(get_registry(), player, key) > 0)
		check_change_level();
	else if (key == 'i')
		ContextSystem::show_entities_list(get_registry(), player);
	else if (key == 'c')
		ContextSystem::show_entity_details(get_registry(), player);
}

void Game::loop()
{
	get_cave().draw();
	UI::instance().print_log(get_registry().ctx().get<GameLogger>().last(Screen::height() / 3));
	UI::instance().update();

	int key = 0;
	while ((key = UI::instance().input(500)) != KEY_ESCAPE)
	{
		handle_key(key);

		get_cave().draw();
		UI::instance().increase_loop_number(); // should do this with registry.ctx() instead
		UI::instance().print_log(get_registry().ctx().get<GameLogger>().last(Screen::height() / 3));

		// if player has used up their action(s), give turn to environment
		// player action is for sure at least moving and attacking and equipping or unequipping
		// environment is all enemies, allies, fire, liquids, whatever
		if (get_registry().get<Actions>(player).actions > 0)
			continue;
	}
	UI::instance().print_log(get_registry().ctx().get<GameLogger>().last(Screen::height()));
}

