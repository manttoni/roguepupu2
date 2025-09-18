#pragma once

#include "Window.hpp"

class InputWindow : public Window
{
	public:
		InputWindow();
		InputWindow(const std::string& label, const std::vector<size_t>& size);
		InputWindow(const Inputwindow& other);
		InputWindow& operator=(const InputWindow& other);
		~InputWindow();
};
