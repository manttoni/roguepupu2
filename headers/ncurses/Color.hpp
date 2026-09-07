#pragma once

#include <array>
#include <string>
#include <regex>
#include <ncurses.h>

namespace Ncurses
{
	struct Color
	{
		int r = 0;
		int g = 0;
		int b = 0;

		Color() = default;
		Color(const int r, const int g, const int b) : r(r), g(g), b(b) {}
		Color(const int i) : r(i), g(i), b(i) {}
		Color(const std::array<int, 3>& rgb) : r(rgb.at(0)), g(rgb.at(1)), b(rgb.at(2)) {}

		bool operator==(const Color& other) const = default;

		short id() const
		{
			return static_cast<short>(r * 36 + g * 6 + b);
		}
		inline std::string markup() const
		{
			return "{" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + "}";
		}
		static inline bool is_markup(const std::string& str, const size_t idx)
		{
			if (str[idx] != '{')
				return false;

			const auto close = str.find('}', idx);
			if (close == std::string::npos)
				return false;

			const auto markup = str.substr(idx, close - idx + 1);
			if (markup == "{reset}")
				return true;
			std::regex regex(R"(\{(\d+),(\d+),(\d+)\})");
			std::smatch match;
			if (!std::regex_match(markup, match, regex))
				return false;


			const int r = std::stoi(match[1].str());
			const int g = std::stoi(match[2].str());
			const int b = std::stoi(match[3].str());

			if (r < 0 || r > 1000 ||
					g < 0 || g > 1000 ||
					b < 0 || b > 1000)
				return false;
			return true;

		}
		static inline Color from_markup(const std::string& str, const size_t idx)
		{
			const auto close = str.find('}', idx);

			const auto markup = str.substr(idx, close - idx + 1);
			std::regex regex(R"(\{(\d+),(\d+),(\d+)\})");
			std::smatch match;
			if (!std::regex_match(markup, match, regex))
				throw std::runtime_error("Invalid color markup: " + markup);

			const int r = std::stoi(match[1].str());
			const int g = std::stoi(match[2].str());
			const int b = std::stoi(match[3].str());

			return Color(r, g, b);
		}

		static inline void init()
		{
			for (size_t r = 0; r < 6; ++r)
			{
				for (size_t g = 0; g < 6; ++g)
				{
					for (size_t b = 0; b < 6; ++b)
					{
						Color c{
							static_cast<int>(r),
								static_cast<int>(g),
								static_cast<int>(b)};
						init_color(c.id(), r * 200, g * 200, b * 200);
						init_pair(c.id(), c.id(), Color(0).id());
					}
				}
			}
		}
	};
}
