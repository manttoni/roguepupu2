#pragma once

#include <filesystem>
#include "ncurses/Color.hpp"

namespace UI
{
	// This only affects the UI colors, game colors are a different thing
	struct Theme
	{
		using Color = Ncurses::Color;

		Color background;
		Color text;
		Color border;
	};

	Theme load_theme(const std::filesystem::path& path = "data/ui/theme.json");
}

