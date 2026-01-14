#include <stddef.h>     // for size_t
#include <any>          // for any
#include <functional>   // for function
#include <string>       // for allocator, basic_string, string
#include "MenuElt.hpp"  // for MenuElt

class MenuTxt : public MenuElt
{
	public:
		inline static std::string HorizontalLine = "{HorizontalLine}";
		MenuTxt(const std::string& text) : MenuElt(text, MenuElt::Type::TEXT) {}
		size_t get_size() const override;
		std::any get_value() const override { return {}; }
		std::function<void()> get_func() const override { return nullptr; }
};
