#include "game/system/System.hpp"
#include "ncurses/Screen.hpp"
#include "game/system/Loot.hpp"
#include "game/system/Lighting.hpp"
#include "game/Simulation.hpp"
#include "game/system/Movement.hpp"
#include "game/component/Component.hpp"
#include "game/world/Position.hpp"
#include "utils/Log.hpp"

// This is temporary
namespace Game::System
{
	template<typename T>
		Event::Node process(
			Simulation&,
			const T&)
		{
			assert(false
				&& "Event has no process() implementation. Implement it!");

			return Event::Node{
				.event = T{},
					.outcome = Event::Outcome::Rejected,
					.consequences = {}
			};
		}
}
// Previous is temporary

/* Rejecting an event means that event makes no changes and produces no
 * consequences.
 * It does not undo its parent.
 *
 * These overloads of 'process' are the main backbone of the ECSystem,
 * but they use helper Systems like System::Movement
 * */
namespace Game::System
{
	using namespace Component;

	Event::Node process(Simulation&, const Event::Null&)
	{
		return Event::Node{.outcome = Event::Outcome::Rejected};
		// this event has no effect on simulation
	}

	Event::Node process(
		Simulation& simulation,
		const Event::Take& event)
	{
		auto& registry = simulation.get_registry();
		using namespace Component;

		const auto* actor_inventory =
			registry.try_get<List::Inventory>(event.entity);
		if (!actor_inventory)
			return Event::Node{.outcome = Event::Outcome::Rejected};

		const auto* container =
			registry.try_get<Component::Value::ContainedIn>(event.item);
		if (!container)
			return Event::Node{.outcome = Event::Outcome::Rejected};

		const auto* container_inventory =
			registry.try_get<List::Inventory>(container->value);
		if (!container_inventory ||
			!container_inventory->contains(event.item))
			return Event::Node{.outcome = Event::Outcome::Rejected};

		return Event::Node::accepted({
			Event::LoseItem{
			.entity = container->value,
			.item = event.item
			},
			Event::ReceiveItem{
			.entity = event.entity,
			.item = event.item
			}
			});
	}
	Event::Node process(
		Simulation& simulation,
		const Event::Drop& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || !inventory->contains(event.item))
			return Event::Node{.outcome = Event::Outcome::Rejected};

		auto* position = registry.try_get<Value::Position>(event.entity);
		if (!position)
			return Event::Node{.outcome = Event::Outcome::Rejected};

		Event::List consequences;

		consequences.push_back(
			Event::LoseItem{
			.entity = event.entity,
			.item = event.item
			});

		consequences.push_back(
			Event::EnterPosition{
			.entity = event.item,
			.position = position->value
			});

		return Event::Node::accepted(consequences);
	}

	Event::Node process(
		Simulation& simulation,
		const Event::LoseItem& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || !inventory->contains(event.item))
			return Event::Node{.outcome = Event::Outcome::Rejected};

		inventory->remove(event.item);
		registry.remove<Component::Value::ContainedIn>(event.item);

		// TODO: add unequip consequence if its equipped

		return Event::Node::accepted();
	}

	std::optional<World::GlobalPosition> get_target_position(
		const entt::registry& registry,
		const Event::Target& target)
	{
		if (auto* position =
			std::get_if<World::GlobalPosition>(&target))
			return *position;

		if (auto* entity =
			std::get_if<entt::entity>(&target);
			entity && registry.valid(*entity))
		{
			if (auto* position =
				registry.try_get<Value::Position>(*entity))
				return position->value;
		}

		return std::nullopt;
	}

	Event::Node process(
		Simulation& simulation,
		const Event::Throw& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || !inventory->contains(event.item))
			return Event::Node{.outcome = Event::Outcome::Rejected};

		const auto target_position =
			get_target_position(registry, event.target);
		if (!target_position)
			return Event::Node{.outcome = Event::Outcome::Rejected};

		Event::List consequences;

		consequences.push_back(
			Event::LoseItem{
			.entity = event.entity,
			.item = event.item
			});

		consequences.push_back(
			Event::EnterPosition{
			.entity = event.item,
			.position = *target_position
			});

		return Event::Node::accepted(consequences);
	}

	Event::Node process(
		Simulation& simulation,
		const Event::ReceiveItem& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || inventory->contains(event.item))
			return Event::Node{.outcome = Event::Outcome::Rejected};

		inventory->values.push_back(event.item);
		registry.emplace<Component::Value::ContainedIn>(
			event.item,
			event.entity);

		return Event::Node::accepted();
	}

	Event::Node process(
		Simulation& simulation,
		const Event::PickUp& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || inventory->contains(event.item))
			return Event::Node{.outcome = Event::Outcome::Rejected};

		auto* item_position =
			registry.try_get<Value::Position>(event.item);
		if (!item_position)
			return Event::Node{.outcome = Event::Outcome::Rejected};

		Event::List consequences;

		consequences.push_back(
			Event::LeavePosition{
			.entity = event.item,
			.position = item_position->value
			});

		consequences.push_back(
			Event::ReceiveItem{
			.entity = event.entity,
			.item = event.item
			});

		return Event::Node::accepted(consequences);
	}

	Event::Node process(
		Simulation& simulation,
		const Event::LoseMovementPoints& event)
	{
		auto& registry = simulation.get_registry();

		auto* movement_points =
			registry.try_get<Resource::MovementPoints>(event.entity);
		if (!movement_points)
			return Event::Node{.outcome = Event::Outcome::Rejected};

		movement_points->current -= event.amount;

		return Event::Node::accepted();
	}

	Event::Node process(
		Simulation& simulation,
		const Event::LeavePosition& event)
	{
		auto& registry = simulation.get_registry();

		Lighting::update(simulation, event.entity, Lighting::Operation::Remove);
		registry.remove<Value::Position>(event.entity);

		return Event::Node::accepted();
	}

	Event::Node process(
		Simulation& simulation,
		const Event::LoadChunks& event)
	{
		std::size_t chunks_loaded = 0;
		const auto viewport = Ncurses::Screen::size();
		const auto dimensions = World::Chunk::dimensions();

		const int radius_y =
			(viewport.y + 2 * dimensions.y - 1) / (2 * dimensions.y);
		const int radius_x =
			(viewport.x + 2 * dimensions.x - 1) / (2 * dimensions.x);

		Event::List events;

		auto& grid = simulation.get_grid();
		auto& generator = simulation.get_generator();
		auto& registry = simulation.get_registry();
		auto& entity_database = simulation.get_entity_database();

		for (int y = event.position.y - radius_y; y <=
			event.position.y + radius_y; ++y)
		{
			for (int x = event.position.x - radius_x; x <=
				event.position.x + radius_x; ++x)
			{
				const World::ChunkPosition cp{y, x};
				if (grid.contains(cp))
					continue;

				grid.add(cp, generator.generate_chunk(cp));
				chunks_loaded++;

				Event::List spawn_events;
				auto& spawn_requests = generator.get_spawn_requests();
				for (const auto& request : spawn_requests)
				{
					const auto entity = Entity::create(
						registry,
						entity_database,
						request.id);
					if (!registry.valid(entity))
					{
						Log::debug() << "Invalid entity " << request.id;
						registry.destroy(entity);
						continue;
					}
					spawn_events.push_back(
						Event::Spawn{
						.entity = entity, // id: request.id
						.position = request.position
						});
				}
				spawn_requests.clear();

				events.insert(
					events.end(),
					std::make_move_iterator(spawn_events.begin()),
					std::make_move_iterator(spawn_events.end()));
			}
		}
		if (chunks_loaded > 0)
			System::Lighting::reset(simulation);

		return Event::Node::accepted(events);
	}

	Event::Node process(
		Simulation& simulation,
		const Event::EnterPosition& event)
	{
		auto& registry = simulation.get_registry();

		if (registry.all_of<Value::Position>(event.entity))
			Log::debug() << registry.get<Value::Name>(event.entity).value <<
				" has position " << event.position;

		registry.emplace<Value::Position>(event.entity, event.position);
		Lighting::update(simulation, event.entity, Lighting::Operation::Add);

		if (event.entity != simulation.get_player())
			return Event::Node::accepted();

		// This is going to include a lot of entity spawn events
		return Event::Node::accepted({
			Event::LoadChunks{.position = World::to_chunk(event.position)}
			});
	}

	Event::Node process(
		Simulation& simulation,
		const Event::Move& event)
	{
		auto& registry = simulation.get_registry();
		const auto& grid = simulation.get_grid();

		const auto* position = registry.try_get<Value::Position>(event.entity);
		if (!position || position->value != event.from)
			throw std::runtime_error("Entity without position cannot move");

		if (!Movement::has_path(registry, grid, position->value, event.to))
			return Event::Node{
				.outcome = Event::Outcome::MovementBlocked};

		const auto movement_cost =
			Movement::cost(simulation, position->value, event.to);

		if (movement_cost >
			registry.get<Component::Resource::MovementPoints>(event.entity)
			.current)
		{
			return Event::Node{
				.outcome = Event::Outcome::InsufficientResources};
		}

		Event::List consequences;

		consequences.push_back(
			Event::LoseMovementPoints{
			.entity = event.entity,
			.amount = movement_cost
			});

		consequences.push_back(
			Event::LeavePosition{
			.entity = event.entity,
			.position = event.from
			});

		consequences.push_back(
			Event::EnterPosition{
			.entity = event.entity,
			.position = event.to
			});

		return Event::Node::accepted(std::move(consequences));
	}

	Event::Node process(
		Simulation& simulation,
		const Event::Bump& event)
	{
		auto& registry = simulation.get_registry();
		auto& grid = simulation.get_grid();

		auto* entity_position =
			registry.try_get<Value::Position>(event.entity);
		if (!entity_position)
			throw std::runtime_error("Entity has no position but is Bumping");

		const auto target_position = entity_position->value + event.direction;

		Event::List consequences;

		if (Movement::has_path(
				registry,
				grid,
				entity_position->value,
				target_position))
		{
			return process(
				simulation,
				Event::Move{
				.entity = event.entity,
				.from = entity_position->value,
				.to = target_position
				});
		}
		else
		{
			return Event::Node{.outcome = Event::Outcome::Rejected};
		}

		return Event::Node::accepted(consequences);
	}

	Event::Node process(
		Simulation& simulation,
		const Event::Spawn& event)
	{
		auto& registry = simulation.get_registry();
		auto& scheduler = simulation.get_scheduler();

		const auto position =
			simulation.get_spawner().find_spawn_position_near(event.entity, event.position);
		if (!position)
		{
			registry.destroy(event.entity);
			return Event::Node{.outcome = Event::Outcome::Rejected};
		}

		if (registry.all_of<Tag::Actor>(event.entity) &&
			!registry.all_of<Tag::Dead>(event.entity))
		{
			scheduler.add(
				Turn::Actor{
				event.entity,
				event.entity == simulation.get_player() ?
				Turn::Controller::Player :
				Turn::Controller::AI,
				1 // TODO: roll initiative calculation
				});
		}

		/* Find position for spawning here, then spawn immediately with
		 * Event::EnterPosition. This will prevent spawning multiple entities
		 * in same cell
		 * */
		std::vector<Event::Node> consequences = {
			simulation.simulate(
				Event::EnterPosition{
					.entity = event.entity,
					.position = *position
			})
		};

		Log::debug() << "Spawning " << registry.get<Component::Value::Name>(
			event.entity).value << " at " << *position;

		/* Spawn loot
		 * */
		const auto& database = simulation.get_entity_database();
		const auto spawn_loot_consequences = Loot::generate(
			registry,
			database,
			event.entity,
			registry.try_get<Component::List::SpawnLoot>(event.entity));

		for (const auto& c : spawn_loot_consequences)
		{
			consequences.push_back(Event::Node{.event = c});
		}
		/* ~Spawn Loot */

		return Event::Node{
			.event = event,
				.outcome = Event::Outcome::Accepted,
				.consequences = consequences};
	}

	Event::Node process(
		Simulation& simulation,
		const Event::BeginTurn& event)
	{
		auto& registry = simulation.get_registry();

		registry.get<Resource::MovementPoints>(event.entity).reset();
		registry.get<Resource::ActionPoints>(event.entity).reset();
		registry.get<Resource::BonusActionPoints>(event.entity).reset();

		return Event::Node::accepted();
	}

	Event::Node process(
		Simulation& simulation,
		const Event::EndTurn& event)
	{
		auto& scheduler = simulation.get_scheduler();
		assert(scheduler.current_actor().entity == event.entity);
		(void) event;
		scheduler.next_turn();

		return Event::Node::accepted({
			Event::BeginTurn{
			.entity = scheduler.current_actor().entity
			}});
	}

	Event::Node dispatch(Simulation& simulation, const Event::Any& event)
	{
		auto node = std::visit(
			[&simulation](const auto& concrete_event)
			{
			return process(simulation, concrete_event);
			},
			event);

		node.event = event;
		return node;
	}
}
