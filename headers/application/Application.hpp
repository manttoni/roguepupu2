#pragma once

#include <optional>

#include "ui/Settings.hpp"
#include "ncurses/Panel.hpp"
#include "game/world/Settings.hpp"
#include "game/Settings.hpp"
#include "game/Simulation.hpp"
#include "databases/EntityDatabase.hpp"
#include "ncurses/Session.hpp"
#include "ncurses/Input.hpp"
#include "rendering/Renderer.hpp"
#include "application/Layout.hpp"

struct Settings
{
		UI::Settings ui;
		Game::Settings game;
};


class History
{
	private:
		std::vector<Game::Event::Node> turns;

	public:
		void add(const Game::Event::Node& root) { turns.push_back(root); }
};

class Messages
{
	private:
		std::vector<std::string> descriptions;

	public:
		void add(const std::vector<std::string>& desc) { descriptions.insert(descriptions.end(), desc.begin(), desc.end()); }
		void add(const std::string& desc) { descriptions.push_back(desc); }
};

class PlayerController
{
	public:
		Game::Event::Any get_event(const Game::Simulation& simulation, const Ncurses::Input::Event& event) const;
};

class Application
{
	private:
		Settings settings;
		EntityDatabase entity_database;

		Ncurses::Session session; // needs only to be created and destroyed

		Layout layout; // has all the ncurses panels
		Renderer renderer; // Renderer.hpp
		History history;
		Messages messages;

		std::optional<Game::Simulation> simulation = std::nullopt;
		PlayerController player_controller; // probably doesnt need to be here but who knows

		bool application_running = true;
		bool game_running = false;
		size_t fps = 2;

		bool handle_input(const Ncurses::Input::Event& event);

		void main_menu();
		void editor_menu();
		void settings_menu();
		void controls_menu();

		void run_game();
		std::string select_player() const;

		Settings load_settings();

	public:
		Application() : renderer(layout.get_game_panel().get_window()) {}
		void run();
};
