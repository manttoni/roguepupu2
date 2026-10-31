#include "game/Simulation.hpp"
#include "game/system/Lighting.hpp"
#include "game/system/Vision.hpp"
#include "game/entity/Entity.hpp"
#include "ncurses/Screen.hpp"

#include "game/system/System.hpp"

#include <deque>
#include <utility>
#include <vector>

namespace Game
{
	Simulation::Simulation(
		const std::string& seed,
		const EntityDatabase& entity_database,
		const Entity::Definition::ID& player_definition_id) :
		seed(seed),
		generator(seed),
		entity_database(entity_database),
		spawner(registry, this->entity_database, grid),
		player(Entity::create(
				registry,
				this->entity_database,
				player_definition_id))
	{
		if (player == entt::null)
			throw std::runtime_error("Player is entt::null");

		load_chunks(World::ChunkPosition{0,0});
		const auto spawn_position =
			spawner.find_spawn_position_near(
				Entity::SpawnRequest{
				player_definition_id,
				World::GlobalPosition{0, 0},
				10.0
				});
		if (!spawn_position)
			throw std::runtime_error("Cannot find suitable player spawn");
		simulate(
			Event::Spawn{
			player,
			*spawn_position
			});
	}

	void Simulation::load_chunks(const World::ChunkPosition& center)
	{
		const auto viewport = Ncurses::Screen::size();
		const auto dimensions = World::Chunk::dimensions();

		const int radius_y =
			(viewport.y + 2 * dimensions.y - 1) / (2 * dimensions.y);
		const int radius_x =
			(viewport.x + 2 * dimensions.x - 1) / (2 * dimensions.x);

		for (int y = center.y - radius_y; y <= center.y + radius_y; ++y)
		{
			for (int x = center.x - radius_x; x <= center.x + radius_x; ++x)
			{
				const World::ChunkPosition cp{y, x};
				if (grid.contains(cp))
					continue;

				grid.add(cp, generator.generate_chunk(cp));

				/* Spawn entities
				 * Spawner checks if it can spawn there and if the cell exists
				 * */
				auto& spawn_requests = generator.get_spawn_requests();
				for (const auto& request : spawn_requests)
				{
					const auto entity = spawner.spawn(request);

					if (entity != entt::null)
						System::Lighting::update(*this, entity);
				}
				spawn_requests.clear();
			}
		}
	}

	void Simulation::load_chunks()
	{
		const auto& player_position =
			registry.get<Component::Value::Position>(player);
		load_chunks(World::to_chunk(player_position.value));
	}

	void Simulation::simulate_node(Event::Node& node)
	{
		assert(!node.outcome.has_value() &&
			"Event already simulated");
		assert(node.consequences.empty() &&
			"Event shouldn't have consequences yet");

		/* Preload grid around player
		 * */
		if (
			const auto* event =
			std::get_if<Event::EnterPosition>(&node.event);
			event && event->entity == player
		   )
		{
			const auto& position =
				registry.get<Game::Component::Value::Position>(player).value;
			const auto chunk_position = World::to_chunk(position);
			load_chunks(chunk_position);
		}
		/* ~Preload
		 * */

		Event::Result result = System::dispatch(*this, node.event);
		node.outcome = result.outcome;

		node.consequences.reserve(result.consequences.size());
		for (auto& consequence : result.consequences)
		{
			Event::Node child;
			child.event = std::move(consequence);
			child.outcome = std::nullopt;
			child.consequences = {};
			simulate_node(child);
			node.consequences.push_back(std::move(child));
		}

		/* TODO: create something so this doesnt get called too often
		 * */
		System::Vision::update_vision(
			registry,
			grid,
			scheduler.current_actor().entity);
	}

	Event::Node Simulation::simulate(Event::Any initial_event)
	{
		Event::Node root;
		root.event = std::move(initial_event);
		root.consequences = {};

		simulate_node(root);

		return root;
	}
}

