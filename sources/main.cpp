#include <ncurses.h>
#include <iostream>
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuBtn.hpp"
#include "UI.hpp"
#include "PerlinNoise.hpp"
#include "Cave.hpp"
#include "CaveGenerator.hpp"
#include "Testing.hpp"

void run()
{
	// Main menu (will organize panels when it becomes clearer)
	//
	// init menu elements (MenuElt)
	std::vector<std::unique_ptr<MenuElt>> elements;
	elements.push_back(std::make_unique<MenuBtn>("CaveGenerator", CaveView::cave_generator));
	elements.push_back(std::make_unique<MenuBtn>("Quit", UI::end_ncurses));

	// init menu with previous elements, and a position
	Menu main_menu(std::move(elements), Screen::middle());

	// moves menu on top and starts its loop
	main_menu.show();
}

int main(void)
{
	UI::init_ncurses();
	test();
	run();
	UI::end_ncurses();
	std::cout << "Success" << std::endl;
	return 0;
}

