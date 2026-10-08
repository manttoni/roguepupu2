#pragma once

#include "game/Enum.hpp"
#include "game/component/Tag.hpp"
#include "game/component/Value.hpp"
#include "game/component/List.hpp"
#include "game/component/Resource.hpp"

namespace Game::Component
{
	inline bool value_is_game_enum(std::string_view component_id)
	{
#define X(name, type) \
		if (component_id == #name) \
		return Game::Enum::GameEnum<type>;

#include "List.def"
#include "Value.def"
#include "Resource.def"
#undef X

		return false;
	}

	std::vector<std::string> get_tag_ids();

	inline std::vector<std::string> get_enum_value_strings(
			std::string_view component_id)
	{
#define X(name, type) \
		if (component_id == #name) \
		return Game::Enum::get_value_strings_or_empty<type>();

#include "List.def"
#include "Value.def"
#include "Resource.def"
#undef X

		return {};
	}
} // namespace Game::Component

