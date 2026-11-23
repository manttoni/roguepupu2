#include "application/PlayerController.hpp"
#include "game/system/Equipment.hpp"
#include "game/Query.hpp"
#include "game/component/List.hpp"
#include "game/component/Runtime.hpp"
#include "ui/Search.hpp"
#include "ui/Menu.hpp"

using namespace Game;

PlayerController::PlayerController(Renderer& renderer) : renderer(renderer) {}

std::optional<World::GlobalPosition> PlayerController::select_position(
	const Simulation& simulation)
{
	const auto& registry = simulation.get_registry();
	const auto player = simulation.get_player();
	const auto player_position =
		registry.get<Component::Value::Position>(player).value;
	auto cursor = player_position;
	while (true)
	{
		renderer.redraw(&simulation, cursor);
		const auto input_event = Ncurses::Input::get_event(500);

		using enum Ncurses::Input::Key;
		switch (input_event.key)
		{
			case None:
				continue;

			case Escape:
				return std::nullopt;

			case Enter:
				return std::optional{cursor};

			default:
				if (input_event.is_directional())
					cursor += input_event.to_direction();
				continue;
		}
	}
}

entt::entity PlayerController::select_entity(
	const Simulation& simulation,
	const std::vector<entt::entity>& entities)
{
	const auto& registry = simulation.get_registry();
	UI::Menu menu("Select Entity");
	for (const auto entity : entities)
	{
		const auto& color =
			registry.get<Component::Value::Color>(entity).value;
		const auto& name =
			registry.get<Component::Value::Name>(entity).value;
		menu.add(UI::Element::Button(color.markup() + name + "{reset}"));
	}
	menu.add(UI::Element::cancel());
	UI::Selection selection = menu.get_selection();
	if (selection.cancelled())
		return entt::null;

	return entities[selection.index];
}

entt::entity PlayerController::select_entity(const Simulation& simulation)
{
	const auto& registry = simulation.get_registry();
	const auto position = select_position(simulation);
	if (!position)
		return entt::null;

	const auto entities =
		Query::find_all(
			registry,
			Component::Value::Position{*position});
	if (entities.empty())
		return entt::null;

	return select_entity(simulation, entities);
}

PlayerController::Interaction PlayerController::get_interaction(
	const Simulation& simulation,
	const entt::entity entity)
{
	if (entity == entt::null)
		return Interaction::Null;

	const auto& registry = simulation.get_registry();
	const auto player = simulation.get_player();

	const auto& player_position =
		registry.get<Component::Value::Position>(player).value;

	const auto location =
		registry.all_of<Component::Value::Position>(entity) ?
		Location::Ground :
		Location::Inventory;

	const auto* contained_in =
		registry.try_get<Component::Value::ContainedIn>(entity);

	std::vector<Interaction> interactions;

	if (registry.all_of<Component::List::Inventory>(entity))
		interactions.push_back(Interaction::OpenInventory);

	std::optional<double> distance =
		location == Location::Ground ?
		std::optional{
			World::distance(
				registry.get<Component::Value::Position>(entity).value,
				player_position)} :
			std::nullopt;

	switch (location)
	{
		case Location::Ground:
			if (*distance < 1.5)
			{
				if (registry.all_of<Component::Tag::Item>(entity))
					interactions.push_back(Interaction::PickUp);
			}
			break;
		case Location::Inventory:
			if (contained_in && contained_in->value == player)
			{
				interactions.push_back(Interaction::Drop);
				interactions.push_back(Interaction::Throw);
				if (registry.all_of<Component::Tag::Equipment>(entity))
				{
					if (System::Equipment::is_equipped(
							registry, player, entity))
						interactions.push_back(Interaction::Unequip);
					else
						interactions.push_back(Interaction::Equip);
				}
			}
			else if (contained_in)
			{
				interactions.push_back(Interaction::Take);
			}
	}

	UI::Menu menu("Interaction");
	for (const auto interaction : interactions)
	{
		menu.add(
			UI::Element::Button(
				std::string(magic_enum::enum_name(interaction))));
	}
	menu.add(UI::Element::cancel());
	const auto selection = menu.get_selection();
	if (selection.cancelled())
		return Interaction::Null;

	return interactions[selection.index];
}

std::optional<Event::Target> PlayerController::select_target(
	const Simulation& simulation,
	Targeting targeting)
{
	const auto position = select_position(simulation);
	if (!position)
		return std::nullopt;

	if (targeting == Targeting::Position)
		return Event::Target{*position};

	const auto& registry = simulation.get_registry();
	const auto entities = Query::find_all(
		registry,
		Component::Value::Position{*position});

	if (entities.empty())
	{
		if (targeting == Targeting::Anything)
			return Event::Target{*position};

		return std::nullopt;
	}

	const auto entity = select_entity(simulation, entities);
	if (entity == entt::null)
		return std::nullopt;

	return Event::Target{entity};
}

Event::Any PlayerController::get_interaction_event(
	const Simulation& simulation)
{
	const auto selected_entity = select_entity(simulation);
	return get_interaction_event(simulation, selected_entity);
}

Event::Any PlayerController::get_interaction_event(
	const Simulation& simulation,
	const entt::entity entity)
{
	const auto interaction = get_interaction(simulation, entity);
	const auto player = simulation.get_player();
	const auto& registry = simulation.get_registry();

	switch (interaction)
	{
		case Interaction::Null:
			return Event::Null{};

		case Interaction::OpenInventory:
			{
				const auto& inventory = registry.get<Component::List::Inventory>(entity).values;
				return get_interaction_event(simulation, select_entity(simulation, inventory));
			}

		case Interaction::PickUp:
			return Event::PickUp{
				.entity = player,
					.item = entity
			};

		case Interaction::Take:
			return Event::Take{
				.entity = player,
					.item = entity
			};

		case Interaction::Throw:
			{
				const auto target = select_target(
					simulation,
					Targeting::Anything);
				if (!target)
					return Event::Null{};

				return Event::Throw{
					.entity = player,
						.item = entity,
						.target = *target
				};
			}

		case Interaction::Drop:
			return Event::Drop{
				.entity = player,
					.item = entity
			};

		case Interaction::Equip:
			return Event::Equip{
				.entity = player,
					.item = entity
			};

		case Interaction::Unequip:
			return Event::Unequip{
				.entity = player,
					.item = entity
			};
	}

	return Event::Null{};
}

Event::Any PlayerController::get_event(
	const Simulation& simulation,
	const Ncurses::Input::Event& event
	)
{
	const auto current_actor = simulation.current_actor();
	assert(current_actor.controller == Turn::Controller::Player);

	const auto player = simulation.get_player();
	assert(player != entt::null);
	assert(current_actor.entity == player);
	(void) current_actor;

	if (event.key == Ncurses::Input::Key::None)
		return Event::Null{};

	if (event.is_directional())
	{
		return Event::Bump{
			.entity = player,
				.direction = event.to_direction()
		};
	}
	else if (event.key == Ncurses::Input::Key::Space)
	{
		return Event::EndTurn{
			.entity = player
		};
	}
	else if (event.key == Ncurses::Input::Key::Enter)
	{
		return get_interaction_event(simulation);
	}

	return Event::Null{};
}
