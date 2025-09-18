#pragma once

#include "Window.hpp"

class OutputWindow : public Window
{
	public:
		OutputWindow();
		OutputWindow(const std::string& label, const std::vector<size_t>& size);
		OutputWindow(const Outputwindow& other);
		OutputWindow& operator=(const OutputWindow& other);
		~OutputWindow();
};
