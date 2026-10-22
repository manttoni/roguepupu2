#include "game/system/System.hpp"
#include "game/Simulation.hpp"
#include "game/system/Movement.hpp"
#include "game/component/Component.hpp"
#include "utils/Log.hpp"

// This is temporary
namespace Game::System
{
	template<typename T>
		Event::Result process(
				Simulation&,
				const T&)
		{
			assert(false
					&& "Event has no process() implementation. Implement it!");

			return {
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
	Event::Result process(Simulation&, const Event::Null&)
	{
		return Event::Result::rejected();
		// this event has no effect on simulation
	}

	Event::Result process(Simulation& simulation, const Game::Event::ReceiveItem& event)
	{
		auto& registry = simulation.get_registry();

		if (
				auto* inventory = registry.try_get<Component::List::Inventory>(event.entity);
				inventory != nullptr)
		{
			inventory->values.push_back(event.item);
		}
		else
			return Event::Result::rejected();

		return Event::Result::accepted();
	}

	Event::Result process(Simulation& simulation, const Game::Event::PickUp& event)
	{
		auto& registry = simulation.get_registry();

		if (!registry.all_of<Component::List::Inventory>(event.entity))
			return Event::Result::rejected();

		registry.erase<Component::Value::Position>(event.item);

		Event::List consequences;

		consequences.push_back(
				Event::ReceiveItem{
					.entity = event.entity,
					.item = event.item
				});

		return Event::Result{
				.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(Simulation& simulation, const Game::Event::LoseMovementPoints& event)
	{
		auto& movement_points =
			simulation.get_registry()
			.get<Component::Resource::MovementPoints>(event.entity);
		movement_points.current -= event.amount;
		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(
			Simulation& simulation,
			const Event::LeavePosition& event)
	{
		(void) simulation;
		(void) event;
		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(
			Simulation& simulation,
			const Event::EnterPosition& event)
	{
		(void) simulation;
		(void) event;
		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(
			Simulation& simulation,
			const Event::Move& event)
	{
		auto& registry = simulation.get_registry();
		assert(event.from
				== registry.get<Component::Value::Position>(event.entity)
				.value);
		const auto& grid = simulation.get_grid();

		if (!Movement::can_move(registry, grid, event.entity, event.to))
			return Event::Result::rejected();

		registry.replace<Component::Value::Position>(
				event.entity,
				event.to);

		Event::List consequences;

		consequences.push_back(
				Event::LoseMovementPoints{
				.entity = event.entity,
				.amount = Vec2{event.from.vec2() - event.to.vec2()}.length()
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

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(
			Simulation& simulation,
			const Event::Bump& event)
	{
		auto& registry = simulation.get_registry();
		auto& grid = simulation.get_grid();
		const auto& entity_position =
			registry.get<Component::Value::Position>(event.entity).value;
		const auto target_position = entity_position + event.direction;
		Event::List consequences;

		if (Movement::can_move(registry, grid, event.entity, target_position))
		{
			consequences.push_back(
					Event::Move{
					.entity = event.entity,
					.from = entity_position,
					.to = target_position
					});
		}
		else
		{
			return Event::Result::rejected();
		}

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(
			Simulation& simulation,
			const Event::Spawn& event)
	{
		auto& registry = simulation.get_registry();
		auto& scheduler = simulation.get_scheduler();

		registry.emplace<Component::Value::Position>(
				event.entity,
				event.position);
		if (registry.all_of<Component::Tag::Actor>(event.entity))
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
				Event::EnterPosition{
				.entity = event.entity,
				.position = event.position
				});

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(
			Simulation& simulation,
			const Event::BeginTurn& event)
	{
		auto& registry = simulation.get_registry();
		namespace Resource = Component::Resource;
		registry.get<Resource::MovementPoints>(event.entity).reset();
		registry.get<Resource::ActionPoints>(event.entity).reset();
		registry.get<Resource::BonusActionPoints>(event.entity).reset();
		return Event::Result::accepted();
	}

	Event::Result process(
			Simulation& simulation,
			const Event::EndTurn& event)
	{
		auto& scheduler = simulation.get_scheduler();
		assert(scheduler.current_actor().entity == event.entity);
		(void) event;
		scheduler.next_turn();

		Event::Result result;
		result.outcome = Event::Outcome::Accepted;

		result.consequences.push_back(
				Event::BeginTurn{
				.entity = scheduler.current_actor().entity
				});
		return result;
	}

	Event::Result dispatch(
			Simulation& simulation,
			const Event::Any& event)
	{
		return std::visit(
				[&](const auto& concrete_event)
				{
				return process(simulation, concrete_event);
				}
				, event);
	}
}
