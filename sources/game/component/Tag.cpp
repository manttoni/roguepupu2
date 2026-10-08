#include "game/component/Tag.hpp"
#include <vector>

namespace Game::Component::Tag
{
	std::vector<std::string> get_ids()
	{
		std::vector<std::string> tag_ids;
#define X(name) \
		tag_ids.push_back(#name);
#include "game/component/Tag.def"
#undef X
		return tag_ids;
	}
}
