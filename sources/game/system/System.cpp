#include "game/system/System.hpp"
#include "game/system/Lighting.hpp"
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
	using namespace Component;

	Event::Result process(Simulation&, const Event::Null&)
	{
		return Event::Result::rejected();
		// this event has no effect on simulation
	}

	Event::Result process(
		Simulation& simulation,
		const Event::Drop& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || !inventory->contains(event.item))
			return Event::Result::rejected();

		auto* position = registry.try_get<Value::Position>(event.entity);
		if (!position)
			return Event::Result::rejected();

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

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(
		Simulation& simulation,
		const Event::LoseItem& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || !inventory->contains(event.item))
			return Event::Result::rejected();

		inventory->remove(event.item);

		// TODO: add unequip consequence if its equipped

		return Event::Result::accepted();
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

	Event::Result process(
		Simulation& simulation,
		const Event::Throw& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || !inventory->contains(event.item))
			return Event::Result::rejected();

		const auto target_position =
			get_target_position(registry, event.target);
		if (!target_position)
			return Event::Result::rejected();

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

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(
		Simulation& simulation,
		const Event::ReceiveItem& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || inventory->contains(event.item))
			return Event::Result::rejected();

		inventory->values.push_back(event.item);

		return Event::Result::accepted();
	}

	Event::Result process(
		Simulation& simulation,
		const Event::PickUp& event)
	{
		auto& registry = simulation.get_registry();

		auto* inventory = registry.try_get<List::Inventory>(event.entity);
		if (!inventory || inventory->contains(event.item))
			return Event::Result::rejected();

		auto* item_position =
			registry.try_get<Value::Position>(event.item);
		if (!item_position)
			return Event::Result::rejected();

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

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = consequences
		};
	}

	Event::Result process(
		Simulation& simulation,
		const Event::LoseMovementPoints& event)
	{
		auto& registry = simulation.get_registry();

		auto* movement_points =
			registry.try_get<Resource::MovementPoints>(event.entity);
		if (!movement_points || movement_points->current < event.amount)
			return Event::Result::rejected();

		movement_points->current -= event.amount;

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(
		Simulation& simulation,
		const Event::LeavePosition& event)
	{
		auto& registry = simulation.get_registry();

		Lighting::update(simulation, event.entity, Lighting::Operation::Remove);
		registry.remove<Value::Position>(event.entity);

		return Event::Result{
			.outcome = Event::Outcome::Accepted,
				.consequences = {}
		};
	}

	Event::Result process(
		Simulation& simulation,
		const Event::EnterPosition& event)
	{
		auto& registry = simulation.get_registry();

		registry.emplace<Value::Position>(event.entity, event.position);
		Lighting::update(simulation, event.entity, Lighting::Operation::Add);

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
		const auto& grid = simulation.get_grid();

		const auto* position = registry.try_get<Value::Position>(event.entity);
		if (!position || position->value != event.from)
			return Event::Result::rejected();

		if (!Movement::can_move(registry, grid, event.entity, event.to))
			return Event::Result::rejected();

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

		auto* entity_position =
			registry.try_get<Value::Position>(event.entity);
		if (!entity_position)
			return Event::Result::rejected();

		const auto target_position = entity_position->value + event.direction;

		Event::List consequences;

		if (Movement::can_move(registry, grid, event.entity, target_position))
		{
			consequences.push_back(
				Event::Move{
				.entity = event.entity,
				.from = entity_position->value,
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
