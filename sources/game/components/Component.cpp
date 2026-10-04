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

namespace Game::Component::Dependency
{
	std::vector<std::string> get_required_tag_ids(
			const std::string_view tag_id)
	{
		std::vector<std::string> ids;

#define X(component, dependency) \
		if constexpr (std::derived_from<dependency, Component::Tag::Base>) \
		{ \
			if (tag_id == component::string) \
			ids.emplace_back(dependency::string); \
		}
#include "game/components/Dependency.def"
#undef X

		return ids;
	}

	std::vector<std::string> get_required_non_tag_ids(
			const std::string_view tag_id)
	{
		std::vector<std::string> ids;

#define X(component, dependency) \
		if constexpr (!std::derived_from<dependency, Component::Tag::Base>) \
		{ \
			if (tag_id == component::string) \
			ids.emplace_back(dependency::string); \
		}
#include "game/components/Dependency.def"
#undef X

		return ids;
	}
}
