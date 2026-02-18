#include <nlohmann/json.hpp>
#include <assert.h>
#include <stddef.h>
#include <nlohmann/json_fwd.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>

#include "domain/Color.hpp"
#include "domain/Conditions.hpp"
#include "domain/Damage.hpp"
#include "domain/Effect.hpp"
#include "generation/CaveGenerator.hpp"
#include "utils/Error.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"

namespace Parser
{
	Color parse_color(const nlohmann::json& data)
	{
		if (data.is_array() && data.size() == 3)
		{
			if (!data[0].is_number() || !data[1].is_number() || !data[2].is_number())
				Error::fatal("Color needs three numeric values");
			const int r = data[0].get<int>();
			const int g = data[1].get<int>();
			const int b = data[2].get<int>();
			if (r < 0 || r > 1000 || g < 0 || g > 1000 || b < 0 || b > 1000)
				Error::fatal("Color value out of bounds [0,1000]");
			return Color(r, g, b);
		}
		if (data.is_object())
		{
			if (!data.contains("red") || !data.contains("green") || !data.contains("blue"))
				Error::fatal("Color needs correct channels");
			if (!data["red"].is_number_integer() || !data["green"].is_number_integer() || !data["blue"].is_number_integer())
				Error::fatal("Color channel value not integer: " + data.dump(4));
			const int r = data["red"].get<int>();
			const int g = data["green"].get<int>();
			const int b = data["blue"].get<int>();
			if (r < 0 || r > 1000 || g < 0 || g > 1000 || b < 0 || b > 1000)
				Error::fatal("Color value out of bounds [0,1000]");
			return Color(r, g, b);
		}
		Error::fatal("Color format is wrong: " + data.dump(4));
	}

	Effect parse_effect(const nlohmann::json& data)
	{
		Effect effect;
		if (data.contains("type"))
		{
			const auto& type = data["type"].get<std::string>();
			if (type == "create_entity")
				effect.type = Effect::Type::CreateEntity;
			else if (type == "destroy_entity")
				effect.type = Effect::Type::DestroyEntity;
			else if (type == "self_destruct")
				effect.type = Effect::Type::SelfDestruct;
			else Error::fatal("Unknown effect type: " + type);
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

	nlohmann::json read_json_file(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
			Error::fatal("Could not open file: " + path.string());
		nlohmann::json defs;
		try {
			file >> defs;
		} catch (const nlohmann::json::parse_error& e) {
			Error::fatal("File: " + path.string() + ", error: " + e.what());
		}
		file.close();
		return defs;
	}

	nlohmann::json parse_json_array(const std::filesystem::path& path)
	{
		auto data = read_json_file(path);
		assert(data.is_array() || data.empty());
		return data;
	}

	void parse_cave_generation_conf(const std::string& conf, CaveGenerator::Data& cgdata)
	{
		const nlohmann::json data = read_json_file("data/generation/cave/conf.json");
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

	Damage::Spec parse_damage_spec(const nlohmann::json& data)
	{
		const auto type_str = data["type"].get<std::string>();
		const auto range = parse_range<size_t>(data["amount"]);
		return Damage::Spec(Damage::string_to_type(type_str), range);
	}

	Random::Perlin parse_perlin(const nlohmann::json& data)
	{
		Random::Perlin perlin;
		perlin.enabled = data["enabled"].get<bool>();
		perlin.frequency = data["frequency"].get<double>();
		perlin.treshold = data["treshold"].get<double>();
		perlin.octaves = data["octaves"].get<size_t>();
		return perlin;
	}
};
