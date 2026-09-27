#include "utils/Log.hpp"
#include "application/Application.hpp"

int main()
{
	Log::info() << "--- Run started ---";

	Application app;
	app.run();

	Log::debug() << "--- Run ended ---";
	return 0;
}

