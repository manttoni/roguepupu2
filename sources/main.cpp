#include <iostream>
#include "PerlinNoise.hpp"
#include "Area.hpp"
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
	Area area(60, 60);
	Log::log("Run succesful\n");
	return 0;
}

