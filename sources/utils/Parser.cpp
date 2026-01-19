#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include "utils/Parser.hpp"
#include "domain/Event.hpp"
#include "domain/Color.hpp"
#include "utils/Log.hpp"
#include "generation/CaveGenerator.hpp"

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
			else if (type == "self_destruct")
				effect.type = Effect::Type::SelfDestruct;
			else Log::error("Unknown effect type: " + type);
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

	nlohmann::json read_file(const std::filesystem::path& path)
	{
		Log::log("Reading " + path.string());
		std::ifstream file(path);
		if (!file.is_open())
			Log::error("Could not open file: " + path.string());
		nlohmann::json defs;
		try {
			file >> defs;
		} catch (const nlohmann::json::parse_error& e) {
			Log::error(e.what());
		}
		file.close();
		return defs;
	}

	void parse_cave_generation_conf(const std::string& conf, CaveGenerator::Data& cgdata)
	{
		const nlohmann::json data = read_file("data/generation/cave/conf.json");
		assert(data.contains(conf));
		const nlohmann::json entry = data[conf];
		const auto dentry = entry["density"];
		const auto eentry = entry["erosion"];
		const auto sentry = entry["smooth"];
		const auto fentry = entry["features"];
		const auto mentry = entry["margin"];
		using namespace CaveGenerator;
		const Data::Density d{dentry["frequency"].get<double>(), dentry["octaves"].get<size_t>()};
		const Data::Erosion e{eentry["erosion_a"].get<double>(), eentry["erosion_b"].get<double>(), eentry["erosion_c"].get<double>()};
		const Data::Smooth s{sentry["intensity"].get<double>(), sentry["iterations"].get<size_t>(), sentry["rock"].get<bool>()};
		const Data::Features f{fentry["sinks"].get<size_t>(), fentry["sources"].get<size_t>()};
		const Data::Margin m{mentry["size"].get<size_t>(), mentry["multiplier"].get<size_t>()};
		cgdata.density = d;
		cgdata.erosion = e;
		cgdata.smooth = s;
		cgdata.features = f;
		cgdata.margin = m;
	}
};
