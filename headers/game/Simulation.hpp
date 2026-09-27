#pragma once

#include <utility>
#include <vector>

#include "external/entt/entt.hpp"
#include "game/world/Grid.hpp"
#include "game/events/Event.hpp"
#include "databases/EntityDatabase.hpp"
#include "game/entities/Entity.hpp"

namespace Game
{
	namespace Turn
	{
		enum class Controller
		{
			Player,
			AI,
		};

		struct Actor
		{
			Actor(	entt::entity entity,
					Controller controller,
					int initiative
					) :
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
				size_t current = 0;

			public:
				Actor current_actor() const { return actors[current]; }
				void update(const entt::registry& registry); // current will stay on the currently acting actor
				void advance();
		};
	}

	class Simulation
	{
		private:
			const std::string seed;
			EntityDatabase entity_database;
			World::Grid world;
			entt::registry registry;
			Turn::Scheduler turn_scheduler;
			entt::entity player;

			void simulate_node(Event::Node& node);

		public:
			Simulation(
					const std::string& seed,
					const EntityDatabase& entity_database,
					const Entity::Definition::ID& player_definition_id);

			const std::string& get_seed() const { return seed; }
			const entt::registry& get_registry() const { return registry; }
			entt::registry& get_registry() { return registry; }
			const World::Grid& get_world() const { return world; }
			const Turn::Scheduler& get_turn_scheduler() const { return turn_scheduler; }
			const EntityDatabase& get_entity_database() const { return entity_database; }
			entt::entity get_player() const { return player; }

			Turn::Actor current_actor() const { return turn_scheduler.current_actor(); }
			Event::Node simulate(Event::Any initial_event);
	};
}
