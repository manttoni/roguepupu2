#include <string>       // for allocator, string
#include "UI/MenuElt.hpp"  // for MenuElt

MenuElt::MenuElt() : text(""), type(MenuElt::Type::NONE) {}
MenuElt::MenuElt(const std::string& text, const MenuElt::Type type) : text(text), type(type) {}

