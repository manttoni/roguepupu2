#include "game/component/Dependency.hpp"

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
#include "game/component/Dependency.def"
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
			{ \
				ids.emplace_back(dependency::string); \
			} \
		}
#include "game/component/Dependency.def"
#undef X

		return ids;
	}
	// Does any tag in the definition directly require this component?
	bool is_directly_required(
		const Entity::Definition& definition,
		const std::string& component_id)
	{
		const auto tags = definition.data.find("Tags");
		if (tags == definition.data.end())
			return false;

		for (const auto& value : *tags)
		{
			const auto& tag_id = value.get_ref<const std::string&>();

			const auto required_tags = get_required_tag_ids(tag_id);
			if (std::find(
					required_tags.begin(),
					required_tags.end(),
					component_id) != required_tags.end())
				return true;

			const auto required_non_tags = get_required_non_tag_ids(tag_id);
			if (std::find(
					required_non_tags.begin(),
					required_non_tags.end(),
					component_id) != required_non_tags.end())
				return true;
		}

		return false;
	}
}
