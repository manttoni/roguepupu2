#pragma once

#include <string>
#include <vector>
#include <string_view>
#include "game/component/Base.hpp"

namespace Game::Component::Tag
{
	std::vector<std::string> get_ids();

	struct Base {};
#define X(name) \
	struct name : Game::Component::Tag::Base \
	{ \
		static constexpr std::string_view string = #name; \
		friend std::ostream& operator<<(std::ostream& os, const name&) \
		{ \
			return os << name::string; \
		} \
	};
#include "Tag.def"
#undef X
}


