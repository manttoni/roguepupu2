#pragma once

#include <optional>

#include "application/PlayerController.hpp"
#include "ncurses/Panel.hpp"
#include "game/world/Settings.hpp"
#include "game/Settings.hpp"
#include "game/Simulation.hpp"
#include "databases/EntityDatabase.hpp"
#include "ncurses/Session.hpp"
#include "ncurses/Input.hpp"
#include "rendering/Renderer.hpp"
#include "application/Layout.hpp"
#include "application/Messages.hpp"

struct Settings
{
	Game::Settings game;
};

class History
{
	private:
		std::vector<Game::Event::Node> turns;

	public:
		void add(const Game::Event::Node& root) { turns.push_back(root); }
};

class Application
{
	private:
		Settings settings;
		EntityDatabase entity_database;

		Ncurses::Session session; // needs only to be created and destroyed

		Layout layout; // has all the ncurses panels
		History history;
		Messages messages;
		Renderer renderer; // Renderer.hpp

		std::optional<Game::Simulation> simulation = std::nullopt;
		PlayerController player_controller;

		bool application_running = true;
		bool game_running = false;
		size_t fps = 2;

		bool handle_input(const Ncurses::Input::Event& event);

		void main_menu();
		void editor_menu();
		void settings_menu();
		void controls_menu();
		void developer_menu();

		void run_game();
		std::string select_player() const;

		Settings load_settings();

	public:
		Application();

		void run();
};
