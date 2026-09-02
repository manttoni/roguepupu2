#include <stddef.h>
#include <string>                      // for basic_string, operator+, opera...
#include <vector>
#include <ncurses.h>
#include <panel.h>

#include "UI/Dialog.hpp"
#include "UI/UI.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/Game.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/action/ActionSystem.hpp"
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
		Log::debug() << "Middle position in game start: " << middle;
		registry.emplace<Position>(player, middle);
	}
}

void Game::select_character(entt::registry& registry)
{
	const auto players = ECS::get_entity_ids<Component::Tag::Player>(registry);
	assert(!players.empty());
	const auto selection = Dialog::get_selection("Select character", players);
	if (selection.cancelled || !selection.element) return;

	registry.ctx().get<GameState>().player = ECS::create_entity(registry, selection.element->label);
}

void Game::loop()
{
	registry.ctx().get<GameState>().game_running = true;
	const auto player = ECS::get_player(registry);
	assert(player != entt::null);
	while (registry.ctx().get<GameState>().game_running && game_over == false)
	{
		Log::info() << "Round " << registry.ctx().get<GameState>().turn_number;
		ActionSystem::act_round(registry, ECS::get_cave(registry, registry.get<Position>(player)).get_idx());
		registry.ctx().get<GameState>().turn_number++;
	}
	if (registry.all_of<Component::Tag::Dead>(player) || game_over)
		Dialog::get_selection("Game over", {"OK"});
}

