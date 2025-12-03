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
#include "EntityFactory.hpp"

void run()
{
	UI::instance().get_menu("main").loop();
}

int main(void)
{
	Log::log("--- Run started ---");
	EntityFactory::instance().init();
	EntityFactory::instance().log_prototypes();
	UI::instance().init();
	run();
	Log::log("Run completed");
	UI::instance().end();
}

