#include <string>
#include <any>
#include "MenuElt.hpp"

class MenuTxt : public MenuElt
{
	public:
		MenuTxt(const std::string& text) : MenuElt(text, MenuElt::Type::TEXT) {}
		size_t get_size() const override { return text.size(); }
		std::any get_value() const override { return {}; }
		void (*get_func() const)() override { return nullptr; }
};
