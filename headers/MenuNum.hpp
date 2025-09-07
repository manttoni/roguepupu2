#pragma once
#include <utility>
#include <algorithm>
#include <string>
#include <limits>
#include <sstream>
#include <any>
#include "MenuElt.hpp"

template <typename T>
class MenuNum : public MenuElt
{
	private:
		std::pair<T, T> limits;
		T value;
		T delta;

	public:
		MenuNum()
			: MenuElt("", "number"), limits({T{}, T{}}), value(T{}), delta(T{}) {}
		MenuNum(
			const std::string& text,
			const std::pair<T, T>& limits = {std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max()},
			const T value = T{0},
			const T delta = T{1}
		)	: MenuElt(text, "number"), limits(limits), value(value), delta(delta)
		{
			if (limits.first > limits.second)
				std::swap(this->limits.first, this->limits.second);
			this->value = std::max(value, this->limits.first);
		}

		std::any get_value() const { return value; }
		void increment() { value = std::min(value + delta, limits.second); }
		void decrement() { value = std::max(value - delta, limits.first); }

		std::string get_text() const override
		{
			std::stringstream ss;
				ss << text << " ";
			ss << (value - delta < limits.first ? "" : "< ");
			ss << value;
			ss << (value + delta > limits.second ? "" : " >");
			return ss.str();
		}
		size_t get_size() const override
		{
			size_t size = text.size() + 5 + std::max(
					std::to_string(limits.first).size(),
					std::to_string(limits.second).size()
				);
			return size;
		}
};
