#pragma once

#include <optional>

#include "external/magic_enum/magic_enum.hpp"
#include "game/event/Event.hpp"
#include "game/Simulation.hpp"
#include "ncurses/Input.hpp"
#include "rendering/Renderer.hpp"

class PlayerController
{
	private:
		enum class Interaction
		{
			Null,
			OpenInventory,
			PickUp,
			Take,
			Throw,
			Drop
		};

		enum class Location
		{
			Ground,
			Inventory
		};

		enum class Targeting
		{
			Anything,
			Entity,
			Position
		};

		Renderer& renderer;

		std::optional<Game::World::GlobalPosition> select_position(
			const Game::Simulation& simulation);

		entt::entity select_entity(
			const Game::Simulation& simulation,
			const std::vector<entt::entity>& entities);

		entt::entity select_entity(
			const Game::Simulation& simulation);

		Interaction get_interaction(
			const Game::Simulation& simulation,
			const entt::entity entity);

		std::optional<Game::Event::Target> select_target(
			const Game::Simulation& simulation,
			Targeting targeting);

		Game::Event::Any get_interaction_event(
			const Game::Simulation& simulation);

		Game::Event::Any get_interaction_event(
			const Game::Simulation& simulation,
			const entt::entity entity);
	public:
		PlayerController(Renderer& renderer);

		Game::Event::Any get_event(
			const Game::Simulation& simulation,
			const Ncurses::Input::Event& event);


	private:

};
