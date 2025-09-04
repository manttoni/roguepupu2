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
#include "Cave.hpp"
#include "Cell.hpp"
#include "PerlinNoise.hpp"

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

	inline double noise3D(double x, double y, double z, double f, int seed)
	{
		static const siv::PerlinNoise::seed_type perlin_seed = seed;
		const siv::PerlinNoise perlin{ perlin_seed };
		return perlin.octave3D_01(x * f, y * f, z * f, 8);
	}
}

