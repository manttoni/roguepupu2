#include "application/PlayerController.hpp"
#include "game/component/List.hpp"
#include "game/component/Runtime.hpp"
#include "ui/Search.hpp"
#include "ui/Menu.hpp"

PlayerController::PlayerController(Renderer& renderer) : renderer(renderer) {}

std::optional<Game::World::GlobalPosition> PlayerController::select_position(
	const Game::Simulation& simulation)
{
	const auto& registry = simulation.get_registry();
	const auto player = simulation.get_player();
	const auto player_position =
		registry.get<Game::Component::Value::Position>(player).value;
	auto cursor = player_position;
	while (true)
	{
		renderer.render_player_perspective(simulation, cursor);
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
	const Game::Simulation& simulation,
	const std::vector<entt::entity>& entities)
{
	const auto& registry = simulation.get_registry();
	UI::Menu menu("Select Entity");
	for (const auto entity : entities)
	{
		const auto& color = registry.get<Game::Component::Value::Color>(entity).value;
		const auto& name = registry.get<Game::Component::Value::Name>(entity).value;
		menu.add(UI::Element::Button(color.markup() + name + "{reset}"));
	}
	menu.add(UI::Element::cancel());
	UI::Selection selection = menu.get_selection();
	if (selection.cancelled())
		return entt::null;

	return entities[selection.index];
}

entt::entity PlayerController::select_entity(const Game::Simulation& simulation)
{
	const auto& registry = simulation.get_registry();
	const auto position = select_position(simulation);
	if (!position)
		return entt::null;

	const auto entities =
		Game::Entity::find_all(
			registry,
			Game::Component::Value::Position{*position});
	if (entities.empty())
		return entt::null;

	return select_entity(simulation, entities);
}

PlayerController::Interaction PlayerController::get_interaction(
	const Game::Simulation& simulation,
	const entt::entity entity)
{
	if (entity == entt::null)
		return Interaction::Null;

	const auto& registry = simulation.get_registry();
	const auto player = simulation.get_player();

	const auto& player_position =
		registry.get<Game::Component::Value::Position>(player).value;

	const auto location =
		registry.all_of<Game::Component::Value::Position>(entity) ?
		Location::Ground :
		Location::Inventory;

	const auto container =
		registry.get<Game::Component::List::Inventory>(player).contains(entity) ?
		player : entt::null; // TODO: figure out whose inventory the entity is in

	std::vector<Interaction> interactions;

	if (registry.all_of<Game::Component::List::Inventory>(entity))
		interactions.push_back(Interaction::OpenInventory);

	std::optional<double> distance =
		location == Location::Ground ?
		std::optional{
			Game::World::distance(
				registry.get<Game::Component::Value::Position>(entity).value,
				player_position)} :
		std::nullopt;

	switch (location)
	{
		case Location::Ground:
			if (*distance < 1.5)
			{
				if (registry.all_of<Game::Component::Tag::Item>(entity))
					interactions.push_back(Interaction::PickUp);
			}
			break;
		case Location::Inventory:
			if (container == player)
			{
				interactions.push_back(Interaction::Drop);
				interactions.push_back(Interaction::Throw);
			}
			else
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

std::optional<Game::Event::Target> PlayerController::select_target(
	const Game::Simulation& simulation,
	Targeting targeting)
{
	const auto position = select_position(simulation);
	if (!position)
		return std::nullopt;

	if (targeting == Targeting::Position)
		return Game::Event::Target{*position};

	const auto& registry = simulation.get_registry();
	const auto entities = Game::Entity::find_all(
		registry,
		Game::Component::Value::Position{*position});

	if (entities.empty())
	{
		if (targeting == Targeting::Anything)
			return Game::Event::Target{*position};

		return std::nullopt;
	}

	const auto entity = select_entity(simulation, entities);
	if (entity == entt::null)
		return std::nullopt;

	return Game::Event::Target{entity};
}

Game::Event::Any PlayerController::get_interaction_event(
	const Game::Simulation& simulation)
{
	const auto selected_entity = select_entity(simulation);
	return get_interaction_event(simulation, selected_entity);
}

Game::Event::Any PlayerController::get_interaction_event(
	const Game::Simulation& simulation,
	const entt::entity entity)
{
	const auto interaction = get_interaction(simulation, entity);
	const auto player = simulation.get_player();
	const auto& registry = simulation.get_registry();

	switch (interaction)
	{
		case Interaction::Null:
			return Game::Event::Null{};

		case Interaction::OpenInventory:
			{
				const auto& inventory = registry.get<Game::Component::List::Inventory>(entity).values;
				return get_interaction_event(simulation, select_entity(simulation, inventory));
			}

		case Interaction::PickUp:
			return Game::Event::PickUp{
				.entity = player,
					.item = entity
			};

		case Interaction::Take:
			return Game::Event::Take{
				.entity = player,
					.item = entity
			};

		case Interaction::Throw:
			{
				const auto target = select_target(
					simulation,
					Targeting::Anything);
				if (!target)
					return Game::Event::Null{};

				return Game::Event::Throw{
					.entity = player,
						.item = entity,
						.target = *target
				};
			}

		case Interaction::Drop:
			return Game::Event::Drop{
				.entity = player,
					.item = entity
			};
	}

	return Game::Event::Null{};
}

Game::Event::Any PlayerController::get_event(
	const Game::Simulation& simulation,
	const Ncurses::Input::Event& event
	)
{
	const auto current_actor = simulation.current_actor();
	assert(current_actor.controller == Game::Turn::Controller::Player);

	const auto player = simulation.get_player();
	assert(player != entt::null);
	assert(current_actor.entity == player);
	(void) current_actor;

	if (event.key == Ncurses::Input::Key::None)
		return Game::Event::Null{};

	if (event.is_directional())
	{
		return Game::Event::Bump{
			.entity = player,
				.direction = event.to_direction()
		};
	}
	else if (event.key == Ncurses::Input::Key::Space)
	{
		return Game::Event::EndTurn{
			.entity = player
		};
	}
	else if (event.key == Ncurses::Input::Key::Enter)
	{
		return get_interaction_event(simulation);
	}

	return Game::Event::Null{};
}
