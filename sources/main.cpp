#include <string>             // for allocator, basic_string
#include "infrastructure/Game.hpp"
#include "database/EntityFactory.hpp"  // for EntityFactory
#include "UI/Menu.hpp"           // for Menu
#include "UI/UI.hpp"             // for UI
#include "utils/Log.hpp"          // for log
#include "testing/Tester.hpp"

/* Capture this from ncurses so it will print */
extern "C" void __assert_fail(
    const char* expr,
    const char* file,
    unsigned int line,
    const char* func)
{
    endwin();
    fprintf(stderr,
        "Assertion failed: %s\nFile: %s:%u\nFunction: %s\n",
        expr, file, line, func);
    abort();
}

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
					"Stealth:   h",
					"DevTools:  `",
					"Hide log:  l"
					});

		if (game != nullptr && game->is_over())
		{
			delete game;
			game = nullptr;
		}
	}
	delete game;
}

int main(int argc, char **argv)
{
	Log::log("--- Run started ---");
	EntityFactory::instance().init();
	if (argc == 2 && std::string(argv[1]) == "test")
	{
		Tester::test();
		return 0;
	}

	UI::instance().init();
	run();
	UI::instance().end();
}

