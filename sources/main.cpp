#include <ncurses.h>
#include <iostream>
#include "Menu.hpp"
#include "UI.hpp"
#include "EntityFactory.hpp"

void run()
{
	UI::instance().get_menu("main").loop();
}

int main(void)
{
	Log::log("--- Run started ---");
	EntityFactory::instance().init();
	UI::instance().init();
	run();
	Log::log("Run completed");
	UI::instance().end();
}

