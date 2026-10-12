#include "ui/Format.hpp"
#include <algorithm>
#include <utility>

namespace UI::Format
{
	std::vector<std::string> elements(const std::vector<Element::Any>& elts)
	{
		std::vector<std::string> lines;
		lines.reserve(elts.size());

		std::size_t label_width = 0;

		static const char separator = ':';

		for (const auto& e : elts)
		{
			auto line = Element::to_string(e);
			const auto colon = line.find(separator);

			if (colon != std::string::npos)
				label_width = std::max(label_width, colon);

			lines.push_back(std::move(line));
		}

		// Align colons and determine the resulting total width.
		std::size_t width = 0;

		for (auto& line : lines)
		{
			const auto colon = line.find(separator);

			if (colon != std::string::npos)
				line.insert(colon, label_width - colon, ' ');

			width = std::max(width, line.size());
		}

		/*
		// Center lines without a colon within the total width.
		for (auto& line : lines)
		{
			if (line.find(separator) == std::string::npos)
				line.insert(0, (width - line.size()) / 2, ' ');
		}
		*/

		return lines;
	}
}
