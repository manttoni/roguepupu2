#include "application/Application.hpp"
#include "game/system/Vision.hpp"
#include "application/EventDescriber.hpp"
#include "application/Layout.hpp"
#include "editor/EntityEditor.hpp"
#include "game/AI.hpp"
#include "game/Simulation.hpp"
#include "game/component/Component.hpp"
#include "ncurses/Input.hpp"
#include "ui/Dialog.hpp"
#include "utils/Log.hpp"

Application::Application() :
	renderer(layout.get_panel(Layout::Panel::Game).get_window()),
	player_controller(renderer)
{}

bool Application::handle_input(const Ncurses::Input::Event& event)
{
	using Key = Ncurses::Input::Key;

	if (event.key == Key::Escape)
		game_running = false;
	else if (event.key == Key::Resize)
	{
		layout.reset();
	}
	else if (event.is_ascii())
	{
		switch (event.ch)
		{
			case '`':
				developer_menu();
				return true;
			case '\\':
				if (simulation.has_value())
				{
					auto& registry = simulation->get_registry();
					auto player = simulation->get_player();
					auto& col =
						registry.get<
						Game::Component::Value::CollisionMovement>(player);
					col.value ^= true;
					return true;
				}
				return false;
			case ']':
				renderer.get_render_all() ^= true;
				return true;
			default:
				return false;
		}
	}
	else
		return false;
	return true;
}

void Application::developer_menu()
{
	auto& registry = simulation->get_registry();
	auto player = simulation->get_player();
	UI::Menu menu("Developer Menu");
	menu.add(
		UI::Element::Checkbox(
			"Movement collision ('\\')",
			&registry.get<Game::Component::Value::CollisionMovement>(player)
			.value));
	menu.add(
		UI::Element::Checkbox(
			"Render all positions (']')",
			&renderer.get_render_all()));
	menu.get_selection();
}

void Application::editor_menu()
{
	EntityEditor::start();
	entity_database = EntityDatabase{};
}

void Application::settings_menu()
{
	UI::Dialog::alert("Not implemented");
}

void Application::controls_menu()
{
	UI::Dialog::alert("Not implemented");
}

void Application::run_game()
{
	layout.get_panel(Layout::Panel::Game).bring_to_top();
	game_running = true;
	assert(simulation.has_value());

	while (game_running)
	{
		Game::System::Vision::update_vision(
			simulation->get_registry(),
			simulation->get_grid(),
			simulation->get_player());
		renderer.render_player_perspective(*simulation);
		renderer.write(messages.get_last(20));
		update_panels();
		doupdate();

		const auto input = Ncurses::Input::get_event(1000 / fps);
		if (handle_input(input))
			continue;

		const auto actor = simulation->current_actor();
		const auto initial_event
			= actor.controller == Game::Turn::Controller::Player
			? player_controller.get_event(*simulation, input)
			: Game::AI::get_event(*simulation, actor.entity)
			;
		auto root = simulation->simulate(initial_event);
		assert(root.outcome.has_value());

		// Auto End Turn - if not in combat, end turn when player runs out of
		// necessary resources
		// If 'root' was rejected, then nothing happened in the simulation
		if (actor.controller == Game::Turn::Controller::Player &&
			*(root.outcome) == Game::Event::Outcome::InsufficientResources)
		{
			root = simulation->simulate(
				Game::Event::EndTurn{.entity = actor.entity});
		}

		// Exact Game::Event history
		history.add(root);

		// Game::Event history saved as strings for player reading
		messages.add(EventDescriber::describe(*simulation, root));

	}
}

// Return the definition id of selected player, simulation will then create it
// TODO: implement this so that can create a new player in place, or use a premade one
std::string Application::select_player() const
{
	return "player_default";
}

void Application::main_menu()
{
	UI::Selection selection;

	while (application_running)
	{
		UI::Menu main("Main Menu");

		if (simulation.has_value())
			main.add(UI::Element::Button("Continue"));

		main.add(UI::Element::Button("New Game"));
		main.add(UI::Element::Button("Editor"));
		main.add(UI::Element::Button("Settings"));
		main.add(UI::Element::Button("Controls"));
		main.add(UI::Element::Button("Exit", UI::Element::Role::Cancel));

		selection = main.get_selection(selection.index);

		if (selection.cancelled()) // Includes ESC keypress
		{
			if (UI::Dialog::confirm("Exit application?"))
			{
				application_running = false;
				return;
			}

			continue;
		}

		if (!selection.selected())
			continue;

		if (selection.label == "Continue")
		{
			run_game();
		}
		else if (selection.label == "New Game")
		{
			if (
				simulation.has_value() &&
				!UI::Dialog::confirm("Lose current game?")
			   )
			{
				continue;
			}

			simulation.emplace(
				UI::Dialog::get_input("Seed"),
				entity_database,
				select_player());
			run_game();
		}
		else if (selection.label == "Editor")
		{
			editor_menu();
		}
		else if (selection.label == "Settings")
		{
			settings_menu();
		}
		else if (selection.label == "Controls")
		{
			controls_menu();
		}
		else
		{
			throw std::runtime_error(
				"Unknown main-menu selection: " +
				selection.label
				);
		}
	}
}

void Application::run()
{
	if (!can_change_color())
	{
		UI::Dialog::alert("Terminal does not support custom color palette");
	}
	while (application_running)
		main_menu();
}
