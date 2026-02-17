#include <assert.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>             // for allocator, basic_string
#include <vector>

#include "infrastructure/Game.hpp"
#include "UI/UI.hpp"             // for UI
#include "utils/Log.hpp"          // for log
#include "UI/Dialog.hpp"
#include "editor/Editor.hpp"
#include "UI/Menu.hpp"

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
	while (true)
	{
		std::vector<std::string> options;
		if (game != nullptr)
			options.push_back("Continue");
		options.push_back("New Game");
		options.push_back("Editor");
		options.push_back("Controls");
		options.push_back("Quit");
		const auto selection = Dialog::get_selection("*** Roguepupu 2 ***", options);
		if (selection.cancelled)
			break;
		assert(selection.element.has_value());
		const auto label = selection.element->label;
		if (label == "Continue" && game != nullptr)
			game->loop();
		else if (label == "New Game")
		{
			delete game;
			game = new Game();
			game->loop();
		}
		else if (label == "Editor")
			Editor::start();
		else if (label == "Controls") // this will get moved in near future
			Dialog::get_selection("Controls", {
					"Movement:     numpad (lock off)",
					"Zoom:         ctrl[+/-] (terminal)",
					"Interact:     left click/enter",
					"Select:       enter",
					"Back:         esc",
					"Inventory:    i",
					"Character:    c",
					"Hide:         h",
					"DevTools:     `",
					"Swap loadout: w"
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

