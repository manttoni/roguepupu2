#include <cassert>
#include "Window.hpp"
#include "InputWindow.hpp"
#include "Utils.hpp"

InputWindow::InputWindow()
{}
InputWindow::InputWindow(const std::string& label, const std::vector<size_t>& size)
	: Window(label, size)
{}
InputWindow::InputWindow(const InputWindow& other)
	: Window(other)
{}
InputWindow& InputWindow::operator=(const InputWindow& other)
{
	if (this != &other)
	{
		Window::operator=(other);
	}
	return *this;
}
InputWindow::~InputWindow()
{}
