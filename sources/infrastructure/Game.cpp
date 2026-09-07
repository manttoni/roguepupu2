#include <stddef.h>
#include <string>                      // for basic_string, operator+, opera...
#include <vector>
#include <ncurses.h>
#include <panel.h>

#include "ui/Dialog.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/Game.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/Action.hpp"
#include "utils/ECS.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"

Game::Game()
{
	ECS::init_registry(registry);
	select_character(registry);
	const auto player = ECS::get_player(registry);
	if (player == entt::null)
		game_over = true;
	else
	{
		CaveGenerator::generate(registry, 0);
		const auto middle = ECS::get_cave(registry, 0).middle_position();
		assert(middle.is_valid());
		registry.emplace<Domain::Position>(player, middle);
	}
}

void Game::select_character(entt::registry& registry)
{
	const auto players = ECS::get_entity_ids<Component::Tag::Player>(registry);
	if (players.empty())
	{
		UI::Dialog::alert("No player characters found.");
		return;
	}
	const auto selection = UI::Dialog::get_selection("Select character", players);
	if (selection.cancelled()) return;

	registry.ctx().get<GameState>().player = ECS::create_entity(registry, players[selection.index]);
}

void Game::loop()
{
	registry.ctx().get<GameState>().game_running = true;
	const auto player = ECS::get_player(registry);
	while (registry.ctx().get<GameState>().game_running && game_over == false)
	{
		Log::info() << "Round " << registry.ctx().get<GameState>().turn_number;
		System::Action::act_round(registry, ECS::get_cave(registry, registry.get<Domain::Position>(player)).get_idx());
		registry.ctx().get<GameState>().turn_number++;
	}
	if (registry.all_of<Component::Tag::Dead>(player) || game_over)
		UI::Dialog::get_selection("Game over", {"OK"});
}

