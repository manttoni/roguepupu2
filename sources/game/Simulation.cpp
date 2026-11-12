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

		simulate(Event::LoadChunks(World::to_chunk({25,25})));

		const auto& node = simulate(Event::Spawn{player, {25, 25}});
		Log::debug() << "Player spawn simulated";
		Log::debug() << node;
		if (grid.get_chunks().empty())
			throw std::runtime_error("Chunks did not get generated");
	}

	// dont use
	Event::List Simulation::initialize()
	{
		Event::List events;
		events.push_back(Event::Spawn{player, {25,25}});
		return events;
	}

	void Simulation::simulate_node(Event::Node& node)
	{
		if (node.outcome)
			return;

		node = System::dispatch(*this, node.event);

		for (auto& child : node.consequences)
			simulate_node(child);
	}

	Event::Node Simulation::simulate(Event::Any initial_event)
	{
		Event::Node root;
		root.event = std::move(initial_event);
		root.outcome = std::nullopt;
		root.consequences = {};

		simulate_node(root);

		return root;
	}
}

