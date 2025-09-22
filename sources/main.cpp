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

void run()
{
	UI::instance().get_menu("main").loop();
}

int main(void)
{
	Log::log("--- Run started ---");
	test();
	UI::instance().init();
	Log::log("Tests passed");
	run();
	Log::log("Run completed");
	UI::instance().end();
}

