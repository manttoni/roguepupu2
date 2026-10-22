#pragma once

#include <map>
#include "ncurses/Panel.hpp"

class Layout
{
	public:
		enum class Panel
		{
			Game
		};
		Layout();
		void reset();

		Ncurses::Panel& get_panel(Panel panel) { return panels[panel]; }

	private:
		std::map<Panel, Ncurses::Panel> panels;
};
