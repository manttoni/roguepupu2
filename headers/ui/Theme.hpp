#pragma once

#include <filesystem>
#include "ncurses/Color.hpp"

namespace UI
{
	struct Theme
	{
		using Color = Ncurses::Color;

		Color background;
		Color text;
		Color border;
		Color rock;
		Color floor;
	};

	Theme load_theme(const std::filesystem::path& path = "data/ui/theme.json");
}
