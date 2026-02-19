#pragma once

#include <limits>
#include <sstream>

template<typename T>
struct Range
{
	static_assert(std::is_arithmetic_v<T>, "Range requires numeric type");
	T min;
	T max;

	static constexpr T min_val = std::numeric_limits<T>::lowest();
	static constexpr T max_val = std::numeric_limits<T>::max();

	Range() : min(T{}), max(T{}) {}
	Range(const T min, const T max) : min(min), max(max) {}

	bool operator==(const Range& other) const = default;

	double average() const
	{
		return (static_cast<double>(min) + static_cast<double>(max)) / 2.0;
	}

	template<typename U>
	bool contains(const U value) const
	{
		static_assert(std::is_arithmetic_v<T>, "Value has to be numeric type");
		return min <= static_cast<T>(value) && max >= static_cast<T>(value);
	}

	friend std::ostream& operator<<(std::ostream& os, const Range& range)
	{
		os << "[" << range.min << "," << range.max << "]";
		return os;
	}

	std::string to_string() const { return std::to_string(min) + " - " + std::to_string(max); }
};
