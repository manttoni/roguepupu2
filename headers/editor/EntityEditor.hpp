#pragma once

#include "entities/Entity.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <nlohmann/json_fwd.hpp>

namespace EntityEditor
{
	using Json = nlohmann::json;
	using Definition = Entity::Definition;

	bool erase_definition(Json& all_data, const Definition& definition);
	bool add_definition(Json& all_data, const Definition& definition);

	std::vector<Definition> filter_definitions(const Json& all_definitions, const std::string& filter);

	std::optional<Definition> search_definition(const Json& all_definitions);
	std::optional<Definition> load_definition(const Json& all_definitions);
	std::optional<Definition> new_definition(const Json& all_definitions);
	void edit_definition(Definition& definition);

	bool definition_matches(const Json& all_definitions, const Definition& definition);
	void activate_definition(Json& all_definitions, Definition& active, const std::optional<Definition>& definition);

	void start();
}
