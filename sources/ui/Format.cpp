#include "ui/Format.hpp"

#include <algorithm>
#include <utility>

#include "ncurses/Color.hpp"
#include "ncurses/Attribute.hpp"

namespace UI::Format
{
	std::size_t visible_width(const std::string& text)
	{
		std::size_t width = 0;

		for (std::size_t i = 0; i < text.size();)
		{
			if (Ncurses::Color::is_markup(text, i))
				i = text.find('}', i) + 1;
			else if (Ncurses::Attribute::is_markup(text, i))
				i = text.find(']', i) + 1;
			else
			{
				++width;
				++i;
			}
		}

		return width;
	}

	std::vector<std::string> elements(const std::vector<Element::Any>& elts)
	{
		std::vector<std::string> lines;
		lines.reserve(elts.size());

		std::size_t label_width = 0;

		for (const auto& element : elts)
		{
			auto line = Element::to_string(element);
			const auto colon = line.find(':');

			if (colon != std::string::npos)
			{
				const auto columns = visible_width(line.substr(0, colon));
				label_width = std::max(label_width, columns);
			}

			lines.push_back(std::move(line));
		}

		for (auto& line : lines)
		{
			const auto colon = line.find(':');

			if (colon != std::string::npos)
			{
				const auto columns = visible_width(line.substr(0, colon));
				line.insert(colon, label_width - columns, ' ');
			}
		}

		return lines;
	}
}
