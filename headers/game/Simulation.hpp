#pragma once

#include <utility>
#include <vector>

#include "external/entt/entt.hpp"
#include "game/world/Grid.hpp"
#include "game/event/Event.hpp"
#include "databases/EntityDatabase.hpp"
#include "game/entity/Entity.hpp"
#include "game/Scheduler.hpp"
#include "game/Settings.hpp"
#include "game/world/Generator.hpp"

namespace Game
{
	class Simulation
	{
		private:
			const std::string seed;

			World::Grid grid;
			World::Generator generator; // or rename Generator

			EntityDatabase entity_database;
			entt::registry registry;

			Turn::Scheduler scheduler;
			entt::entity player;

			Settings settings;

			void simulate_node(Event::Node& node);

		public:
			Simulation(
					const std::string& seed,
					const EntityDatabase& entity_database,
					const Entity::Definition::ID& player_definition_id);

			const std::string& get_seed() const { return seed; }

			const entt::registry& get_registry() const { return registry; }
			entt::registry& get_registry() { return registry; }

			const World::Grid& get_grid() const { return grid; }
			World::Grid& get_grid() { return grid; }

			const Turn::Scheduler& get_scheduler() const { return scheduler; }
			Turn::Scheduler& get_scheduler() { return scheduler; }

			const EntityDatabase& get_entity_database() const { return entity_database; }

			entt::entity get_player() const { return player; }

			const Settings& get_settings() const { return settings; }
			Settings& get_settings() { return settings; }

			Turn::Actor current_actor() const { return scheduler.current_actor(); }
			Event::Node simulate(Event::Any initial_event);
	};
}
