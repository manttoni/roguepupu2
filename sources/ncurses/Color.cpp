#include <nlohmann/json.hpp>
#include <stdexcept>
#include "ncurses/Color.hpp"

namespace Ncurses
{
	Color::Color(const int id)
	{
		if (id < 16 || id > 231)
		{
			throw std::out_of_range("Color id must be 16–231, but is: " + std::to_string(id));
		}

		const int index = id - 16;
		r = index / 36;
		g = (index / 6) % 6;
		b = index % 6;
	}

	void from_json(const nlohmann::json& data, Color& color)
	{
		if (data.is_number_integer())
		{
			const auto number = data.get<int>();
			if (number < 16 || number > 231)
				throw std::invalid_argument(
						"Color id must be [16,231]");
			color = Color{number};
			return;
		}
		else if (!data.is_array() || data.size() != 3)
			throw std::invalid_argument(
					"Color must be an array of exactly 3 integers");

		for (const auto& channel : data)
		{
			if (!channel.is_number_integer()
					|| channel < 0 || channel > 5)
			{
				throw std::invalid_argument(
						"Color channels must be integers from 0 to 5");
			}
		}

		color.r = data.at(0).get<int>();
		color.g = data.at(1).get<int>();
		color.b = data.at(2).get<int>();
	}
	std::ostream& operator<<(std::ostream& os, const Color& color)
	{
		os <<
			"red: " << color.r << ", " <<
			"green: " << color.g << ", " <<
			"blue: " << color.b;
		return os;
	}
}
