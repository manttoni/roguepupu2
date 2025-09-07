#include "MenuBtn.hpp"
#include "MenuElt.hpp"

MenuBtn::MenuBtn() : MenuElt("", MenuElt::Type::BUTTON), func(nullptr) {}
MenuBtn::MenuBtn(const std::string& text, void (*func)()) : MenuElt(text, MenuElt::Type::BUTTON), func(func) {}
MenuBtn::~MenuBtn() {}

size_t MenuBtn::get_size() const
{
	return text.size();
}
