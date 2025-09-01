#pragma once

#include <cmath>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>
#include "Area.hpp"
#include "Cell.hpp"

namespace Utils
{
	template <typename T>
	void remove_element(std::vector<T> &vec, const T &elt)
	{
		vec.erase(std::remove(vec.begin(), vec.end(), elt), vec.end());
	}
	template <typename T>
	bool contains(const std::vector<T> &vec, const T &elt)
	{
		return std::find(vec.begin(), vec.end(), elt) != vec.end();
	}
}

namespace Log
{
	const std::string logfile = "logs/logs.log";
	static inline std::string timestamp()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);
		std::tm tm = *std::localtime(&time);

		std::ostringstream oss;
		oss << std::put_time(&tm, "[%d.%m-%Y %H:%M:%S]");
		return oss.str();
	}

	static inline void log(const std::string& message)
	{
		std::ofstream os(logfile, std::ios::app);
		if (!os)
			throw std::runtime_error("Can't open " + logfile);

		os << timestamp() << " " << message << std::endl;
	}
}

namespace Random
{
	inline std::mt19937 &rng()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		return gen;
	}

	inline int randint(const int min, const int max)
	{
		std::uniform_int_distribution<> dist(min, max);
		return dist(rng());
	}
}

namespace Colors {
    // Basic ANSI 8 colors
    constexpr char RESET[]   = "\033[0m";
    constexpr char RED[]     = "\033[31m";
    constexpr char GREEN[]   = "\033[32m";
    constexpr char YELLOW[]  = "\033[33m";
    constexpr char BLUE[]    = "\033[34m";
    constexpr char MAGENTA[] = "\033[35m";
    constexpr char CYAN[]    = "\033[36m";
    constexpr char WHITE[]   = "\033[37m";
	constexpr char BLACK[] = "\033[30m";

    // Bright variants
    constexpr char BRIGHT_RED[]   = "\033[91m";
    constexpr char BRIGHT_GREEN[] = "\033[92m";

    // True color example function
    inline std::string rgb(int r, int g, int b) {
        return "\033[38;2;" + std::to_string(r) + ";" +
               std::to_string(g) + ";" + std::to_string(b) + "m";
    }
}

