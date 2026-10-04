#include "game/components/Component.hpp"

namespace Game::Component
{
	std::vector<std::string> get_tag_ids()
	{
		std::vector<std::string> tag_ids;
#define X(name) \
		tag_ids.push_back(#name);
#include "game/components/Tag.def"
#undef X
		return tag_ids;
	}
}
