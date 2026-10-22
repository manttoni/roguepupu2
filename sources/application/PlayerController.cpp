#include "application/PlayerController.hpp"
#include "game/component/Runtime.hpp"
#include "ui/Search.hpp"
#include "ui/Menu.hpp"

Game::Event::Any PlayerController::select_entity_interaction_event(
		const entt::registry& registry,
		const entt::entity entity,
		const entt::entity player)
{
	namespace Component = Game::Component;
	namespace Value = Component::Value;

	//const bool in_player_inventory =
	//	registry.get<Component::List::Inventory>(player).contains(entity);
	const bool on_ground =
		registry.all_of<Component::Value::Position>(entity);
	const auto& entity_name =
		registry.get<Component::Value::Color>(entity).value.markup()
		+ registry.get<Component::Value::Name>(entity).value
		+ "{reset}";

	UI::Menu menu(entity_name);
	std::vector<std::string> buttons;

	if (registry.all_of<Component::Tag::Item>(entity))
	{
		if (on_ground)
			buttons.push_back("Pick Up");
	//	else if (in_player_inventory)
	//		buttons.push_back("Drop");
		else
			buttons.push_back("Take");
	}
	if (registry.all_of<Component::List::Inventory>(entity))
	{
		buttons.push_back("Inventory");
	}

	for (const auto& button : buttons)
		menu.add(UI::Element::Button{button});
	menu.add(UI::Element::cancel());

	const auto selection = menu.get_selection();
	if (selection.cancelled())
		return Game::Event::Null{};

	if (selection.label == "Pick Up")
	{
		return Game::Event::PickUp{
			.entity = player,
				.item = entity
		};
	}
	else if (selection.label == "Drop")
	{
		return Game::Event::Drop{
			.entity = player,
				.item = entity
		};
	}
	else if (selection.label == "Take")
	{
		return Game::Event::Take{
			.entity = player,
				.item = entity
		};
	}
	else if (selection.label == "Inventory")
	{
		return select_entity_list_interaction_event(
				registry,
				registry.get<Component::List::Inventory>(entity).values,
				player);
	}
	return Game::Event::Null{};
}

Game::Event::Any PlayerController::select_entity_list_interaction_event(
		const entt::registry& registry,
		const std::vector<entt::entity>& entities,
		const entt::entity player)
{
	if (entities.size() == 1)
		return select_entity_interaction_event(registry, entities.back(), player);

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
					select_entity_interaction_event(registry, entity, player);
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
	const auto player = simulation.get_player();
	const auto* const cell = grid.find_cell(position);
	if (cell == nullptr)
		throw std::runtime_error("Cell not found");

	const auto entities = Game::Entity::find_all(
			registry,
			Game::Component::Value::Position{position});

	if (entities.empty())
		return Game::Event::Null{};

	return select_entity_list_interaction_event(registry, entities, player);
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
