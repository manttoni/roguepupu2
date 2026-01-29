#pragma once

#include <vector>
#include <string>
#include "utils/Vec2.hpp"
#include "utils/Screen.hpp"
#include "UI/Menu.hpp"

/* For making dialog boxes asking for user input or showing a message
 * */
namespace Dialog
{
	/* Leave options empty to just show a message,
	 * otherwise will block until user selects an option.
	 * If only showing message, it will disappear when next update happens.
	 * */
	Menu::Element get_selection(
			const std::vector<std::string>& text,
			const std::vector<std::string>& buttons = {},
			const Vec2& position = Screen::middle());

};
