#include <iostream>
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

int main(void)
{
	Log::log("Running main");
	test();
	Cave first_level = CaveGenerator::generate_cave();
	Cave second_level = CaveGenerator::generate_cave(first_level);
	first_level.print_cave();
	second_level.print_cave();
	Log::log("Run succesful\n");
	return 0;
}

