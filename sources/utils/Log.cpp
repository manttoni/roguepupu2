#include "utils/Log.hpp"

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace Log
{
	Stream::Stream(const char* level)
		: level(level)
	{
	}

	Stream::~Stream() noexcept
	{
		try
		{
			std::filesystem::create_directories("logs");

			std::ofstream file("logs/logs.log", std::ios::app);
			if (!file)
			{
				std::fputs("Could not open logs/logs.log\n", stderr);
				return;
			}

			const std::time_t now = std::time(nullptr);
			const std::tm* time = std::localtime(&now);

			if (time)
				file << std::put_time(time, "[%d.%m.%Y %H:%M:%S]") << ' ';

			file << level << ' ' << buffer.str() << '\n';
		}
		catch (...)
		{
			// A destructor must not throw, especially during exception handling.
			std::fputs("Could not write to logs/logs.log\n", stderr);
		}
	}

	Stream info()    { return Stream{"[INFO]"}; }
	Stream debug()   { return Stream{"[DEBUG]"}; }
	Stream warning() { return Stream{"[WARNING]"}; }
	Stream error()   { return Stream{"[ERROR]"}; }
}
