#include "game/Simulation.hpp"

#include "game/systems/System.hpp"

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
		entity_database(entity_database),
		world(seed),
		player(Entity::create(registry, entity_database, player_definition_id))
	{
		if (player == entt::null)
			throw std::runtime_error("Player is entt::null");

		simulate(
				Event::Spawn{
					player,
					World::GlobalPosition{0, 0} // TODO: make some spawning logic for player
				});
	}

	void Simulation::simulate_node(Event::Node& node)
	{
		assert(!node.outcome.has_value() &&
				"Event already simulated");
		assert(node.consequences.empty() &&
				"Event shouldn't have consequences yet");

		Event::Result result = System::dispatch(*this, node.event);

		node.outcome = result.outcome;

		node.consequences.reserve(result.consequences.size());
		for (auto& consequence : result.consequences)
		{
			Event::Node child{
				.event = std::move(consequence),
				.outcome = std::nullopt,
				.consequences = {}
			};

			simulate_node(child);
			node.consequences.push_back(std::move(child));
		}
	}

	Event::Node Simulation::simulate(Event::Any initial_event)
	{
		Event::Node root{
			.event = std::move(initial_event),
			.consequences = {}
		};

		simulate_node(root);
		return root;
	}
}
