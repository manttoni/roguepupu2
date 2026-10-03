#include "game/systems/System.hpp"
#include "game/Simulation.hpp"
#include "game/systems/Movement.hpp"
#include "game/components/Component.hpp"
#include "utils/Log.hpp"

// This is temporary
namespace Game::System
{
	template<typename T>
		Event::Result process(
				Simulation&,
				const T&)
		{
			assert(false && "Event has no process() implementation. Implement it!");

			return {
				.outcome = Event::Outcome::Rejected,
					.consequences = {}
			};
		}
}
// Previous is temporary

/* Rejecting an event means that event makes no changes and produces no consequences.
 * It does not undo its parent.
 *
 * These overloads of 'process' are the main backbone of the ECSystem,
 * but they use helper Systems like System::Movement
 * */
namespace Game::System
{
	Event::Result process(Simulation&, const Game::Event::Null&)
	{
		return Event::Result::rejected(); // this event has no effect on simulation
	}

	Event::Result process(Simulation& simulation, const Game::Event::LoseMovementPoints& event)
	{
		auto& movement_points = simulation.get_registry().get<Game::Component::Resource::MovementPoints>(event.entity);
		movement_points.current -= event.amount;
		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(Simulation& simulation, const Game::Event::LeavePosition& event)
	{
		(void) simulation;
		(void) event;
		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(Simulation& simulation, const Game::Event::EnterPosition& event)
	{
		(void) simulation;
		(void) event;
		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(Simulation& simulation, const Game::Event::Move& event)
	{
		auto& registry = simulation.get_registry();
		assert(event.from == registry.get<Component::Value::Position>(event.entity).value);
		const auto& grid = simulation.get_grid();

		if (!Movement::can_move(registry, grid, event.entity, event.to))
			return Event::Result::rejected();

		registry.replace<Game::Component::Value::Position>(event.entity, event.to);

		Event::List consequences;

		consequences.push_back(
				Game::Event::LoseMovementPoints{
				.entity = event.entity,
				.amount = Vec2{event.from.vec2() - event.to.vec2()}.length()
				});

		consequences.push_back(
				Game::Event::LeavePosition{
				.entity = event.entity,
				.position = event.from
				});

		consequences.push_back(
				Game::Event::EnterPosition{
				.entity = event.entity,
				.position = event.to
				});

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(Simulation& simulation, const Game::Event::Bump& event)
	{
		auto& registry = simulation.get_registry();
		auto& grid = simulation.get_grid();
		const auto& entity_position = registry.get<Game::Component::Value::Position>(event.entity).value;
		const auto target_position = entity_position + event.direction;
		Event::List consequences;

		if (Movement::can_move(registry, grid, event.entity, target_position))
		{
			consequences.push_back(
					Game::Event::Move{
					.entity = event.entity,
					.from = entity_position,
					.to = target_position
					});
		}
		else
		{
			return Event::Result::rejected(); // TODO: for now, only consequence is moving
		}

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(Simulation& simulation, const Game::Event::Spawn& event)
	{
		auto& registry = simulation.get_registry();
		auto& scheduler = simulation.get_scheduler();

		// TODO: validate position. Extract from System::Movement the blocks_movement() and make a System::Spatial?

		registry.emplace<Game::Component::Value::Position>(event.entity, event.position);
		if (registry.all_of<Game::Component::Tag::Actor>(event.entity))
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

		Event::List consequences;

		consequences.push_back(
				Game::Event::EnterPosition{
				.entity = event.entity,
				.position = event.position
				});

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(Simulation& simulation, const Game::Event::BeginTurn& event)
	{
		auto& registry = simulation.get_registry();
		registry.get<Game::Component::Resource::MovementPoints>(event.entity).reset();
		registry.get<Game::Component::Resource::ActionPoints>(event.entity).reset();
		registry.get<Game::Component::Resource::BonusActionPoints>(event.entity).reset();
		return Event::Result::accepted();
	}

	Event::Result process(Simulation& simulation, const Game::Event::EndTurn& event)
	{
		auto& scheduler = simulation.get_scheduler();
		assert(scheduler.current_actor().entity == event.entity);
		(void) event;
		scheduler.next_turn();

		Event::Result result;
		result.outcome = Event::Outcome::Accepted;

		result.consequences.push_back(
				Game::Event::BeginTurn{
				.entity = scheduler.current_actor().entity
				});
		return result;
	}

	Game::Event::Result dispatch(Simulation& simulation, const Game::Event::Any& event)
	{
		return std::visit(
				[&](const auto& concrete_event)
				{
				return process(simulation, concrete_event);
				}
				, event);
	}
}
