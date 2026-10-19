#include "application/PlayerController.hpp"
#include "ui/Search.hpp"
#include "ui/Menu.hpp"

Game::Event::Any PlayerController::select_entity_interaction_event(
		const entt::registry& registry,
		const entt::entity entity)
{
	const auto& entity_name =
		registry.get<Game::Component::Value::Name>(entity).value;
	UI::Menu menu(entity_name);
	menu.add(UI::Element::cancel());
	const auto selection = menu.get_selection();
	if (selection.cancelled())
		return Game::Event::Null{};
	return Game::Event::Null{};
}

Game::Event::Any PlayerController::select_entity_list_interaction_event(
		const entt::registry& registry,
		const std::vector<entt::entity>& entities)
{
	if (entities.size() == 1)
		return select_entity_interaction_event(registry, entities.back());

	UI::Selection selection;
	UI::Menu menu;
	for (const auto entity : entities)
	{
		menu.add(
				UI::Element::Button(
					registry.get<Game::Component::Value::Name>(entity).value));
	}
	menu.add(UI::Element::cancel());
	while (!selection.cancelled())
	{
		selection = UI::Search::select(menu);
		if (selection.cancelled())
			break;

		assert(selection.selected());

		const auto& entity_name = selection.label;

		for (const auto entity : entities)
		{
			if (entity_name
					== registry.get<Game::Component::Value::Name>(entity).value)
			{
				const auto event =
					select_entity_interaction_event(registry, entity);
				return event;
			}
		}
	}
	return Game::Event::Null{};
}

Game::Event::Any PlayerController::select_position_interaction_event(
		const Game::Simulation& simulation,
		const Game::World::GlobalPosition& position)
{
	const auto& grid = simulation.get_grid();
	const auto& registry = simulation.get_registry();
	const auto* const cell = grid.find_cell(position);
	if (cell == nullptr)
		throw std::runtime_error("Cell not found");

	const auto entities = Game::Entity::find_all(
			registry,
			Game::Component::Value::Position{position});

	if (entities.empty())
		return Game::Event::Null{};

	return select_entity_list_interaction_event(registry, entities);
}

Game::Event::Any PlayerController::get_event(
		const Game::Simulation& simulation,
		const Ncurses::Input::Event& event
		) const
{
	const auto current_actor = simulation.current_actor();
	assert(current_actor.controller == Game::Turn::Controller::Player);
	const auto player = simulation.get_player();
	assert(player != entt::null);
	assert(current_actor.entity == player);

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
		return select_position_interaction_event(
				simulation,
				simulation.get_registry()
				.get<Game::Component::Value::Position>(player).value);
	}

	return Game::Event::Null{};
}
