#include <assert.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>             // for allocator, basic_string
#include <vector>

#include "ncurses/Ncurses.hpp"
#include "infrastructure/Game.hpp"
#include "utils/Log.hpp"          // for log
#include "ui/Dialog.hpp"
#include "editor/EntityEditor.hpp"
#include "ui/Menu.hpp"

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

	Log::error() << "Assertion failed: " << expr << " in file: " << file << ":" << line << " in function: " << func;
	abort();
}

void run()
{
	Game* game = nullptr;
	UI::Selection selection;
	while (true)
	{
		std::vector<std::string> options;
		if (game != nullptr)
			options.push_back("Continue");
		options.push_back("New Game");
		options.push_back("Entity Editor");
		options.push_back("Controls");
		options.push_back("Quit");
		selection = UI::Dialog::get_selection("*** Roguepupu 2 ***", options, Ncurses::Screen::middle(), selection.index);
		if (selection.cancelled())
			break;
		const auto label = options[selection.index];
		if (label == "Continue" && game != nullptr)
			game->loop();
		else if (label == "New Game")
		{
			delete game;
			game = new Game();
			game->loop();
		}
		else if (label == "Entity Editor")
			EntityEditor::start();
		else if (label == "Controls")
			UI::Dialog::get_selection("Controls", {
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
	Log::info() << "--- Run started ---";

	Ncurses::init();
	Ncurses::Color::init();
	run();
	Ncurses::end();
}

