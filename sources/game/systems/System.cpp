#include "game/systems/System.hpp"
#include "game/systems/Movement.hpp"
#include "game/components/Component.hpp"

// This is temporary
namespace Game::System
{
	template<typename T>
	Event::Result process(
			Simulation&,
			const T&)
	{
		assert(false && "Event has no process() implementation");

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
	Event::Result process(Simulation& simulation, const Game::Event::LoseMovementPoints& event)
	{
		auto& movement_points = simulation.get_registry().get<Component::Resource::MovementPoints>(event.entity).current;
		movement_points -= event.amount;
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

		if (!Movement::can_move(simulation, event.entity, event.to))
		{
			return Event::Result::rejected();
		}

		registry.replace<Component::Value::Position>(event.entity, event.to);

		Event::List consequences;

		consequences.push_back(
				Game::Event::LoseMovementPoints{
					.entity = event.entity,
					.amount = Game::World::GlobalPosition{event.from - event.to}.length()
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
