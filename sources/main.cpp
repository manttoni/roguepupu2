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
	std::string selection = "none";
	while (!selection.empty() && selection != "Quit")
	{
		std::vector<std::string> options;
		if (game != nullptr)
			options.push_back("Continue");
		options.push_back("New Game");
		options.push_back("Controls");
		options.push_back("Quit");
		selection = UI::instance().dialog("Roguepupu 2", options);
		if (selection == "Continue" && game != nullptr)
		{
			game->loop();
		}
		else if (selection == "New Game")
		{
			game = new Game();
			game->loop();
		}
		else if (selection == "Controls")
		{
			UI::instance().dialog("Controls", {
					"Move with arrows or WASD",
					"Interact with left click",
					"Use menu with arrows or mouse",
					"i: inventory",
					"c: character"});
		}
	}
}

int main(int argc, char** argv)
{
	Log::log("--- Run started ---");
	EntityFactory::instance().init();
	if (argc > 1 && std::string(argv[1]) == "test")
	{
		tester();
		return 0;
	}
	UI::instance().init();
	run();
	UI::instance().end();
}

