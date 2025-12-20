#include <thread>
#include <chrono>
#include <string>                      // for basic_string, operator+, opera...
#include "Cave.hpp"                    // for Cave
#include "Cell.hpp"                    // for Cell
#include "Components.hpp"              // for Actions, Position
#include "ECS.hpp"                     // for get_cell, can_see, get_colored...
#include "EntityFactory.hpp"           // for EntityFactory
#include "Game.hpp"                    // for Game
#include "GameLogger.hpp"              // for GameLogger
#include "Renderer.hpp"                // for Renderer
#include "UI.hpp"                      // for UI, KEY_ESCAPE, KEY_LEFT_CLICK
#include "Utils.hpp"                   // for log, top, Vec2
#include "World.hpp"                   // for World
#include "entt.hpp"                    // for vector, allocator, basic_sigh_...
#include "systems/ContextSystem.hpp"   // for show_entities_list, show_entit...
#include "systems/MovementSystem.hpp"  // for move

Game::Game() :
	level(1),
	registry(world.get_registry()),
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
	else if (key == ' ')
	{
		auto& actions = registry.get<Actions>(player);
		actions.used = actions.actions;
		registry.ctx().get<GameLogger>().log(ECS::get_colored_name(registry, player) + " does nothing");
	}
}

void Game::reset_actions()
{
	auto actors = get_registry().view<Actions>();
	for (const auto& actor : actors)
	{
		auto& actions = get_registry().get<Actions>(actor);
		actions.used = 0;
	}
}

void Game::environment_turn()
{
	auto actors = get_cave().get_creature_cache();
	const auto player_idx = ECS::get_cell(registry, player)->get_idx();
	for (const auto actor : actors)
	{
		if (!ECS::can_see(registry, actor, player))
			continue;

		const auto actor_idx = ECS::get_cell(registry, actor)->get_idx();
		const auto& path = get_cave().find_path(actor_idx, player_idx);
		if (path.empty())
			continue;

		const auto move_to = path[1];
		Vec2 direction = get_cave().get_direction(actor_idx, move_to);
		MovementSystem::move(get_registry(), actor, direction);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		Renderer::render(get_cave());
	}
}

void Game::loop()
{
	Renderer::render(get_cave());
	int key = 0;
	while ((key = UI::instance().input(500)) != KEY_ESCAPE)
	{
		handle_key(key);

		Renderer::render(get_cave());
		UI::instance().increase_loop_number(); // should do this with registry.ctx() instead

		if (ECS::has_actions_left(get_registry(), player))
			continue;
		environment_turn();
		if (ECS::is_dead(registry, player))
		{
			UI::instance().dialog("Game Over", {"OK"}, Screen::top());
			over = true;
			return;
		}
		reset_actions();
	}
}

