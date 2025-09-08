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
#include <ncurses.h>
#include "Cave.hpp"
#include "Cell.hpp"
#include "PerlinNoise.hpp"

namespace Screen
{
	struct Coord { size_t y, x; };

	inline size_t height()	{ return static_cast<size_t>(LINES); }
	inline size_t width()	{ return static_cast<size_t>(COLS); }

	inline Coord middle()		{ return {height() / 2, width() / 2 }; }
	inline Coord top_left()	{ return {0, 0}; }
	inline Coord top()			{ return {0, width() / 2}; }
} // namespace Screen

// remove this and replace with c++20 versions
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

namespace Math
{
	inline double map(double x, double a, double b, double c, double d)
	{
		return c + (x - a) * (d - c) / (b - a);
	}

	// increment safely
	template <typename T> void increment(T& value, const T& max_limit)
	{
		if (value > max_limit - T{1})
		{
			value = max_limit;
			return;
		}
		value = value + T{1};
	}
	template <typename T> void decrement(T& value, const T& min_limit)
	{
		if (value < min_limit + T{1})
		{
			value = min_limit;
			return;
		}
		value = value - T{1};
	}
}

namespace Random
{
	inline std::mt19937& rng()
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

	inline double randreal(const double min, const double max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return dist(rng());
	}

	inline double noise3D(double x, double y, double z, double f, int seed, int octave)
	{
		const siv::PerlinNoise::seed_type perlin_seed = seed;
		const siv::PerlinNoise perlin{ perlin_seed };
		return perlin.octave3D_01(x * f, y * f, z * f, octave);
	}
}

