#pragma once

#include "game/component/Component.hpp"
#include "game/entity/Definition.hpp"
#include <string>
#include <nlohmann/json.hpp>


namespace Game::Component::Dependency
{
	std::vector<std::string> get_required_tag_ids(std::string_view tag_id);
	std::vector<std::string> get_required_non_tag_ids(std::string_view tag_id);

	// Does C directly require D?
	template<typename C, typename D>
		constexpr bool directly_requires()
		{
#define X(component, dependency) \
			if constexpr (std::same_as<C, component> && std::same_as<D, dependency>) \
			return true;
#include "Dependency.def"
#undef X

			return false;
		}
	bool is_directly_required(
			const Game::Entity::Definition& definition,
			const std::string& component_id);


}
