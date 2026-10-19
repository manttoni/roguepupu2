#pragma once

#include "game/event/Event.hpp"
#include "game/Simulation.hpp"
#include "ncurses/Input.hpp"

class PlayerController
{
	private:
		static Game::Event::Any select_entity_interaction_event(
				const entt::registry& registry,
				const entt::entity entity,
				const entt::entity player);

		static Game::Event::Any select_entity_list_interaction_event(
				const entt::registry& registry,
				const std::vector<entt::entity>& entities,
				const entt::entity player);

		static Game::Event::Any select_position_interaction_event(
				const Game::Simulation& simulation,
				const Game::World::GlobalPosition& position);

	public:
		Game::Event::Any get_event(
				const Game::Simulation& simulation,
				const Ncurses::Input::Event& event) const;
};
