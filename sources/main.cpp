#include <ncurses.h>
#include <iostream>
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuBtn.hpp"
#include "MenuNum.hpp"
#include "UI.hpp"
#include "PerlinNoise.hpp"
#include "Cave.hpp"
#include "CaveGenerator.hpp"
#include "Testing.hpp"

void quit()
{
	UI::instance().end();
}

void run()
{
	// Debug window
	std::vector<std::unique_ptr<MenuElt>> debug_elements;
	debug_elements.push_back(std::make_unique<MenuNum<int>>("Colors"));
	debug_elements.push_back(std::make_unique<MenuNum<int>>("Color pairs"));
	Menu debug(std::move(debug_elements), Screen::botleft());

	// test
	debug.set_value("Colors", 10);

	// This menu will just display numbers, so dont wait for any input
	debug.set_read_only(true);
	debug.loop();
	UI::instance().update();

	// Main menu
	std::vector<std::unique_ptr<MenuElt>> elements;
	elements.push_back(std::make_unique<MenuBtn>("CaveGenerator", CaveView::cave_generator));
	elements.push_back(std::make_unique<MenuBtn>("Quit", quit));
	Menu main_menu(std::move(elements), Screen::middle());

	UI::instance().add_menu("main", &main_menu);
	UI::instance().add_menu("debug", &debug);

	// moves menu on top and starts its loop
	main_menu.loop();
}

int main(void)
{
	UI::instance().init();
	test();
	run();
	quit();
	std::cout << "Success" << std::endl;
	return 0;
}

