#include "Area.hpp"
#include "Testing.hpp"
#include <iostream>

int main(void)
{
	Log::log("Running main");
	test();
	Area area(25,25);
	area.print_area();
	Log::log("Run succesful\n");
	return 0;
}

