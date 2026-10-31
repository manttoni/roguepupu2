#pragma once

#include <utility>
#include <vector>

#include "databases/EntityDatabase.hpp"
#include "external/entt/entt.hpp"
#include "game/Scheduler.hpp"
#include "game/Settings.hpp"
#include "game/entity/Entity.hpp"
#include "game/entity/Spawner.hpp"
#include "game/event/Event.hpp"
#include "game/world/Generator.hpp"
#include "game/world/Grid.hpp"

namespace Game
{
	class Simulation
	{
		private:
			const std::string seed;

			World::Grid grid;
			World::Generator generator;

			EntityDatabase entity_database;
			entt::registry registry;

			Entity::Spawner spawner;

			Turn::Scheduler scheduler;
			entt::entity player;

			Settings settings;

			void simulate_node(Event::Node& node);

			struct LightColor
			{
				double r = 0.0;
				double g = 0.0;
				double b = 0.0;
			};
			using Lightmap =
				std::unordered_map<
				Game::World::GlobalPosition,
				LightColor>;
			Lightmap lightmap;

		public:
			Simulation(
				const std::string& seed,
				const EntityDatabase& entity_database,
				const Entity::Definition::ID& player_definition_id);
			Simulation(const Simulation&) = delete;
			Simulation& operator=(const Simulation&) = delete;

			Simulation(Simulation&&) = delete;
			Simulation& operator=(Simulation&&) = delete;

			const std::string& get_seed() const { return seed; }

			const entt::registry& get_registry() const { return registry; }
			entt::registry& get_registry() { return registry; }

			const World::Grid& get_grid() const { return grid; }
			World::Grid& get_grid() { return grid; }

			const Turn::Scheduler& get_scheduler() const { return scheduler; }
			Turn::Scheduler& get_scheduler() { return scheduler; }

			const EntityDatabase& get_entity_database() const
			{ return entity_database; }

			entt::entity get_player() const { return player; }

			const Settings& get_settings() const { return settings; }
			Settings& get_settings() { return settings; }

			Turn::Actor current_actor() const
			{ return scheduler.current_actor(); }

			Event::Node simulate(Event::Any initial_event);
			void load_chunks(const World::ChunkPosition& center);
			void load_chunks();

			Lightmap& get_lightmap() { return lightmap; }
			const Lightmap& get_lightmap() const { return lightmap; }
	};
}
