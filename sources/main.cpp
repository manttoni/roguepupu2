#include <iostream>
#include "UI.hpp"
#include "PerlinNoise.hpp"
#include "Cave.hpp"
#include "CaveGenerator.hpp"
#include "Testing.hpp"

/*void test_perlin()
{
	int height = 60;
	int width = 120;
	for (int i = 0; i < height * width; ++i)
	{
		int y = i / width;
		int x = i % width;
		double perlin = Random::noise(y, x, 1, 8);
		int rounded = static_cast<int>(std::round(Math::map(perlin, 0, 1, 1, 5)));
		std::cout << rounded << " ";
		if (x == width - 1)
			std::cout << std::endl;
	}
}*/

void run()
{
	UI::init_ncurses();
	UI::start_menu();
	UI::end_ncurses();
}

int main(void)
{
	Log::log("Running main");
	test();
	run();
	Log::log("Run succesful\n");
	return 0;
}

