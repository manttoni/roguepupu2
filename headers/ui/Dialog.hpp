#pragma once

#include <vector>
#include <string>
#include "utils/Vec2.hpp"
#include "ncurses/Screen.hpp"
#include "ui/Menu.hpp"

/* For making dialog boxes asking for user input or showing a message
 * */
namespace UI::Dialog
{
	/* Leave options empty to just show a message,
	 * otherwise will block until user selects an option.
	 * If only showing message, it will disappear when next update happens.
	 * */
	UI::Selection get_selection(
			const std::vector<std::string>& text,
			const std::vector<std::string>& buttons = {},
			const Vec2<int>& position = Ncurses::Screen::middle(),
			const size_t default_selected = 0
			);
	UI::Selection get_selection(
			const std::string& text,
			const std::vector<std::string>& buttons = {},
			const Vec2<int>& position = Ncurses::Screen::middle(),
			const size_t default_selected = 0
			);
	void message(const std::string& message, const Vec2<int>& position = Ncurses::Screen::middle());
	void message(const std::vector<std::string>& messages, const Vec2<int>& position = Ncurses::Screen::middle());
	bool confirm(const std::string& message);
	void alert(const std::string& message);
	void get_input(const std::string& label, std::string* input);
};
