#include "MenuBtn.hpp"
#include "MenuElt.hpp"

MenuBtn::MenuBtn() : MenuElt("", "button"), func(nullptr) {}
MenuBtn::MenuBtn(const std::string& text, void (*func)()) : MenuElt(text, "button"), func(func) {}
MenuBtn::~MenuBtn() {}

size_t MenuBtn::get_size() const
{
	return text.size();
}
