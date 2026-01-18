#include <stddef.h>     // for size_t
#include <any>          // for any
#include <functional>   // for function
#include <string>       // for allocator, basic_string, string
#include "UI/MenuElt.hpp"  // for MenuElt
#include "UI/MenuTxt.hpp"

size_t MenuTxt::get_size() const
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
