#include <cassert>
#include "Window.hpp"
#include "OutputWindow.hpp"
#include "Utils.hpp"

OutputWindow::OutputWindow()
{}
OutputWindow::OutputWindow(const std::string& label, const std::vector<size_t>& size)
	: Window(label, size)
{}
OutputWindow::OutputWindow(const OutputWindow& other)
	: Window(other)
{}
OutputWindow& OutputWindow::operator=(const OutputWindow& other)
{
	if (this != &other)
	{
		Window::operator=(other);
	}
	return *this;
}
OutputWindow::~OutputWindow()
{}
