#pragma once

#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <stdexcept>

namespace Log
{
	const std::string logfile = "logs/logs.log";
	static inline std::string timestamp()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);
		std::tm tm = *std::localtime(&time);
		std::ostringstream oss;
		oss << std::put_time(&tm, "[%d.%m.%Y %H:%M:%S]");
		return oss.str();
	}

	static inline void log(const std::string& message, const std::string& filename = logfile)
	{
		std::ofstream os(filename, std::ios::app);
		if (!os)
			throw std::runtime_error("Can't open " + filename);

		os << timestamp() << " " << message << std::endl;
		os.close();
	}

	[[noreturn]] static inline void error(const std::string& message)
	{
		log(message);
		throw std::runtime_error("Error: " + message);
	}
}

