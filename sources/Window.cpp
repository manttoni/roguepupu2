#include <cassert>
#include "Window.hpp"
#include "Utils.hpp"

OutputWindow::OutputWindow()
{}
OutputWindow::OutputWindow(const std::string& label, const std::vector<size_t>& size)
	: label(label), height(size[0]), width(size[1]), y(size[2]), x(size[3]),
	panel(new_panel(newwin(height, width, y, x)))
{
	assert(panel != nullptr);
	assert(y + height <= Screen::height());
	assert(x + width <= Screen::width());
}
OutputWindow::OutputWindow(const OutputWindow& other)
	: label(other.label), height(other.height), width(other.width), y(other.y), x(other.x), panel(other.panel)
{
	other.panel = nullptr;
}
OutputWindow& OutputWindow::operator=(const OutputWindow& other)
{
	if (this != &other)
	{
		label = other.label;
		height = other.height;
		width = other.width;
		y = other.y;
		x = other.x;
		panel = other.panel;
		other.panel = nullptr;
	}
	return *this;
}
OutputWindow::~OutputWindow()
{
	delwin(panel_window(panel));
	del_panel(panel);
}
