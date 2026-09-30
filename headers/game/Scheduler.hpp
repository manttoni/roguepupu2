#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

#include "external/entt/entt.hpp"

namespace Game::Turn
{
	enum class Controller
	{
		Player,
		AI,
	};

	struct Actor
	{
		Actor(
				entt::entity entity,
				Controller controller,
				int initiative) :
			entity(entity),
			controller(controller),
			initiative(initiative)
		{
			assert(entity != entt::null);
		}

		entt::entity entity;
		Controller controller;
		int initiative;
	};

	class Scheduler
	{
		private:
			std::vector<Actor> actors;
			std::size_t current = 0;

		public:
			Actor current_actor() const { return actors[current]; }
			void next_turn();
			void add(Actor actor);
	};
}

