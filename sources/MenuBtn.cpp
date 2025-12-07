#include <string>
#include "MenuBtn.hpp"
#include "MenuElt.hpp"

MenuBtn::MenuBtn() : MenuElt("", MenuElt::Type::BUTTON), func(nullptr) {}
MenuBtn::MenuBtn(const std::string& text, void (*func)()) : MenuElt(text, MenuElt::Type::BUTTON), func(func) {}
MenuBtn::~MenuBtn() {}

size_t MenuBtn::get_size() const
{
	size_t markup_len = 0;
	size_t pos = 0;
	// Return the length of text, but subtract length of possible color markup
	while (text.find('{', pos) != std::string::npos && text.find('}', pos) != std::string::npos) // contains markups
	{
		markup_len += text.find('}', pos) - text.find('{', pos);
		pos = text.find('}', pos) + 1;
	}
	return text.size() - markup_len;
}
