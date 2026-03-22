#include <stddef.h>
#include <string>                      // for basic_string, operator+, opera...
#include <vector>
#include <ncurses.h>
#include <panel.h>

#include "UI/UI.hpp"
#include "database/EntityFactory.hpp"           // for EntityFactory
#include "domain/Cave.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/Game.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/environment/LiquidSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"
#include "UI/Dialog.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "domain/World.hpp"
#include "infrastructure/GameState.hpp"

struct Dead;

Game::Game()
{
	ECS::init_registry(registry);
	select_character(registry);
	const auto player = ECS::get_player(registry);
	if (player == entt::null)
		game_over = true;
	else
	{
		CaveGenerator::generate(registry, 0, true);
		registry.emplace<Position>(player, ECS::get_cave(registry, 0).middle_position());
	}
}

void Game::select_character(entt::registry& registry)
{
	const nlohmann::json player_filter = {
		{ "contains_all", {{ "tags", {"player"}}}} // must include 'tags' and it must include 'player'
	};
	const auto players = EntityFactory::instance().filter_entity_ids(player_filter);
	assert(!players.empty());
	const auto selection = Dialog::get_selection("Select character", players);
	if (selection.cancelled || !selection.element) return;

	registry.ctx().get<GameState>().player = EntityFactory::instance().create_entity(registry, selection.element->label);
}

void Game::loop()
{
	registry.ctx().get<GameState>().game_running = true;
	const auto player = ECS::get_player(registry);
	while (registry.ctx().get<GameState>().game_running && game_over == false)
	{
		Log::log("Round " + std::to_string(registry.ctx().get<GameState>().turn_number));
		ActionSystem::act_round(registry, ECS::get_cave(registry, registry.get<Position>(player)).get_idx());
		LiquidSystem::simulate_liquids(registry);
		registry.ctx().get<GameState>().turn_number++;
	}
	if (registry.all_of<Dead>(player))
		game_over = true;
	if (game_over)
		Dialog::get_selection("Game over", {"OK"});
}

