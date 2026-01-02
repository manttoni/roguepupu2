#include <string>             // for allocator, basic_string
#include "Game.hpp"
#include "EntityFactory.hpp"  // for EntityFactory
#include "Menu.hpp"           // for Menu
#include "UI.hpp"             // for UI
#include "Utils.hpp"          // for log
#include "Tester.hpp"

void run()
{
	Game* game = nullptr;
	std::string selection = "";
	while (selection != "Quit")
	{
		std::vector<std::string> options;
		if (game != nullptr)
			options.push_back("Continue");
		options.push_back("New Game");
		options.push_back("Controls");
		options.push_back("Quit");
		selection = UI::instance().dialog("*** Roguepupu 2 ***", options);
		if (selection == "Continue" && game != nullptr)
			game->loop();
		else if (selection == "New Game")
		{
			delete game;
			game = new Game();
			game->loop();
		}
		else if (selection == "Controls")
			UI::instance().dialog("Controls", {
					"Movement:  numpad (lock off)",
					"Zoom:      ctrl[+/-] (terminal)",
					"Interact:  left click",
					"Select:    enter",
					"Back:      esc",
					"Inventory: i",
					"Character: c",
					"Hide:      h",
					"DevTools:  `"
					});

		if (game != nullptr && game->is_over())
		{
			delete game;
			game = nullptr;
		}
	}
	delete game;
}

int main(void)
{
	Log::log("--- Run started ---");
	EntityFactory::instance().init();
	UI::instance().init();
	run();
	UI::instance().end();
}

