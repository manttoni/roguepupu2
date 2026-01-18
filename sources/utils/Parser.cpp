#include <nlohmann/json.hpp>
#include "utils/Parser.hpp"
#include "domain/Event.hpp"
#include "domain/Color.hpp"
#include "utils/Log.hpp"

namespace Parser
{
	Color parse_color(const nlohmann::json& data)
	{
		if (!data.is_array() || data.size() != 3)
			Log::error("Data not right sized array: " + data.dump(4));
		Color color = Color(data[0].get<int>(), data[1].get<int>(), data[2].get<int>());
		return color;
	}

	Effect parse_effect(const nlohmann::json& data)
	{
		Effect effect;
		if (data.contains("type"))
		{
			const auto& type = data["type"].get<std::string>();
			if (type == "create_entity")
				effect.type = Effect::Type::CreateEntity;
			else if (type == "transition")
				effect.type = Effect::Type::Transition;
			else if (type == "destroy_entity")
				effect.type = Effect::Type::DestroyEntity;
			else Log::error("Uknown effect type: " + type);
		}
		if (data.contains("entity_id"))
			effect.entity_id = data["entity_id"].get<std::string>();
		return effect;
	}
	Conditions parse_conditions(const nlohmann::json& data)
	{
		Conditions conditions;
		if (data.contains("category"))
			conditions.category = data["category"].get<std::string>();
		if (data.contains("category_not"))
			conditions.category_not = data["category_not"].get<std::string>();
		if (data.contains("weight_min"))
			conditions.weight_min = data["weight_min"].get<double>();
		if (data.contains("weight_max"))
			conditions.weight_max = data["weight_max"].get<double>();

		return conditions;
	}

	std::pair<size_t, size_t> parse_range(const nlohmann::json& data)
	{
		if (data.is_number())
		{
			const size_t n = data.get<size_t>();
			return {n, n};
		}
		if (data.is_string())
		{
			const auto str = data.get<std::string>();
			auto pos = str.find('-');
			if (pos == std::string::npos)
				Log::error("Invalid range: " + str);
			const size_t min = std::stoi(str.substr(0, pos));
			const size_t max = std::stoi(str.substr(pos + 1));
			if (min > max)
				Log::error("Invalid range: " + str);
			return {min, max};
		}
		Log::error("Unsupported range format: " + data.dump(4));
	}
};
