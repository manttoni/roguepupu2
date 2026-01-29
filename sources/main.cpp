#include <string>             // for allocator, basic_string
#include "infrastructure/Game.hpp"
#include "database/EntityFactory.hpp"  // for EntityFactory
#include "UI/Menu.hpp"           // for Menu
#include "UI/UI.hpp"             // for UI
#include "utils/Log.hpp"          // for log

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

	Log::log("Assertion failed: " + std::string(expr) + " in file: " + file + ":" + std::to_string(line) + " in function: " + func);
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
		Log::log("Selection: " + selection);
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
					"Interact:  left click/enter",
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

int main()
{
	Log::log("--- Run started ---");

	UI::instance().init();
	run();
	UI::instance().end();
}

