#include "utils/Log.hpp"
#include "application/Application.hpp"
#include <stdexcept>
#include <iostream>

int main()
{
	Log::info() << "--- Run started ---";

	try
	{
		Application application;
		application.run();
	}
	catch (const std::exception& error)
	{
		// Application has unwound, so Session has restored the terminal.
		Log::error() << error.what();
		std::cerr << "Fatal error: " << error.what() << '\n';
		return EXIT_FAILURE;
	}

	Log::info() << "--- Run ended ---";
	return 0;
}

