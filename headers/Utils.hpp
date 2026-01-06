#pragma once

#include <bits/chrono.h>    // for system_clock
#include <curses.h>         // for COLS, LINES
#include <cctype>           // for toupper
#include <ctime>            // for size_t, localtime, time_t, tm
#include <fstream>          // for char_traits, basic_ostream, basic_ofstream
#include <iomanip>          // for operator<<, put_time
#include <random>           // for uniform_int_distribution, mt19937, random...
#include <sstream>          // for basic_ostringstream
#include <stdexcept>        // for runtime_error
#include <string>           // for basic_string, string, operator+, operator<<
#include <vector>           // for allocator, vector
#include "PerlinNoise.hpp"  // for PerlinNoise

enum class Direction
{
	DOWN,
	UP,
	LEFT,
	RIGHT
};

struct Vec2
{
	int y, x;

	bool operator!=(const Vec2& other) const { return y != other.y || x != other.x; }
	bool operator==(const Vec2& other) const { return y == other.y && x == other.x; }
};

namespace Screen
{
	struct Coord { size_t y, x; };

	inline size_t height()	{ return static_cast<size_t>(LINES); }
	inline size_t width()	{ return static_cast<size_t>(COLS); }

	inline Coord middle()		{ return {height() / 2, width() / 2 }; }
	inline Coord top()			{ return {0, width() / 2}; }
	inline Coord botleft()		{ return {height(), 0}; }
	inline Coord topright()		{ return {0, width()}; }
	inline Coord topleft()		{ return {0, 0}; }
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
	inline std::string capitalize(const std::string& str)
	{
		std::string cap = str;
		cap[0] = std::toupper(cap[0]);
		size_t pos = 0;
		while ((pos = str.find(' ', pos)) != std::string::npos)
		{
			pos++;
			cap[pos] = std::toupper(cap[pos]);
		}
		return cap;
	}
}

namespace Log
{
	enum class Type
	{
		FATAL_ERROR,
		INFO,
		TEST_FAIL
	};
	const std::string logfile = "logs/logs.log";
	const std::string testerlogfile = "logs/tester.log";
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

	static inline void tester_log(const Type type, const std::string& message)
	{
		log(message, testerlogfile);
		if (type == Type::FATAL_ERROR)
			throw std::runtime_error("Fatal error: " + message);
	}
}

namespace Math
{
	template <typename T> static inline T clamp(const T& value, const T& min, const T& max)
	{
		return std::min(max, std::max(min, value));
	}
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

	inline Vec2 polar_to_cartesian(const Vec2 center, const double radius, const double angle)
	{
		const size_t y = static_cast<size_t>(std::round(center.y - radius * std::sin(angle)));
		const size_t x = static_cast<size_t>(std::round(center.x + radius * std::cos(angle)));
		return Vec2(y, x);
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

	// INT
	inline int randint(const int min, const int max, const size_t seed)
	{
		std::mt19937 gen(seed);
		std::uniform_int_distribution<int> dist(min, max);
		return dist(gen);
	}
	inline int randint(const int min, const int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng());
	}

	// SIZE_T
	inline size_t randsize_t(const size_t min, const size_t max, std::mt19937& rng_obj = rng())
	{
		std::uniform_int_distribution<size_t> dist(min, max);
		return dist(rng_obj);
	}

	// DOUBLE
	inline double randreal(const double min, const double max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return dist(rng());
	}
	inline double randreal(const double min, const double max, const size_t seed)
	{
		std::mt19937 gen(seed);
		std::uniform_real_distribution<> dist(min, max);
		return dist(gen);
	}

	// PERLIN NOISE
	inline double noise3D(double x, double y, double z, double f, int seed, int octave)
	{
		const siv::PerlinNoise::seed_type perlin_seed = seed;
		const siv::PerlinNoise perlin{ perlin_seed };
		return perlin.octave3D_01(x * f, y * f, z * f, octave);
	}
}

