#include "application/Application.hpp"
#include "utils/Log.hpp"
#include "game/Simulation.hpp"
#include "application/EventDescriber.hpp"
#include "ui/Dialog.hpp"
#include "ncurses/Input.hpp"
#include "game/AI.hpp"
#include "application/Layout.hpp"

Game::Event::Any PlayerController::get_event(
		const Game::Simulation& simulation,
		const Ncurses::Input::Event& event
		) const
{
	const auto current_actor = simulation.current_actor();

	assert(current_actor.controller == Game::Turn::Controller::Player);

	if (event.key == Ncurses::Input::Key::None)
		return Game::Event::Null{};

	if (event.is_directional())
	{
		return Game::Event::Bump{
			.entity = current_actor.entity,
			.direction = event.to_direction()
		};
	}
	else if (event.key == Ncurses::Input::Key::Space)
	{
		return Game::Event::EndTurn{
			.entity = current_actor.entity
		};
	}
	return Game::Event::Null{};
}

bool Application::handle_input(const Ncurses::Input::Event& event)
{
	if (event.key == Ncurses::Input::Key::Escape)
		game_running = false;
	if (event.key == Ncurses::Input::Key::Resize)
		layout.reset();
	else
		return false;
	return true;
}

void Application::editor_menu()
{
	UI::Dialog::alert("Not implemented");
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
	game_running = true;
	assert(simulation.has_value());

	while (game_running)
	{
		const auto input = Ncurses::Input::get_event(1000 / fps);
		if (handle_input(input))
			continue;

		const auto actor = simulation->current_actor();
		const auto initial_event
			= actor.controller == Game::Turn::Controller::Player
			? player_controller.get_event(*simulation, input)
			: Game::AI::get_event(*simulation, actor.entity)
			;
		const auto root = simulation->simulate(initial_event);
		assert(root.outcome.has_value());

		history.add(root);
		messages.add(EventDescriber::describe(*simulation, root));

		renderer.render(*simulation);
		//renderer.render(messages);
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
		main.add(UI::Element::Button("Exit", UI::Element::Button::Role::Cancel));

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

			simulation.emplace(Game::Simulation(UI::Dialog::get_input("Seed"), entity_database, select_player()));
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
	while (application_running)
		main_menu();
}
