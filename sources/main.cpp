#include <iostream>
#include "UI.hpp"
#include "PerlinNoise.hpp"
#include "Cave.hpp"
#include "CaveGenerator.hpp"
#include "Testing.hpp"

void run()
{
	UI::init_ncurses();
	UI::start_menu();
	UI::end_ncurses();
}

int main(void)
{
	test();
	run();
	return 0;
}

