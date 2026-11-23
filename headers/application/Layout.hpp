#pragma once

#include <map>
#include "ncurses/Panel.hpp"
#include "utils/Vec2.hpp"

class Layout
{
	public:
		struct Settings
		{
			// Position of the panel junction in terminal cells: {row, column}.
			Vec2<int> cross_position{};
		};

		enum class Panel
		{
			Game,
			MessageLog,
			PlayerStatus
		};
		Layout();
		explicit Layout(const Settings& settings);
		void reset();

		Settings& get_settings() { return settings; }
		const Settings& get_settings() const { return settings; }
		Ncurses::Panel& get_panel(Panel panel) { return panels[panel]; }

	private:
		Settings settings;
		std::map<Panel, Ncurses::Panel> panels;
};
