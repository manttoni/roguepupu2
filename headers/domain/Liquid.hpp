#pragma once

#include <utility>
#include <string>
#include <map>
#include "domain/Color.hpp"
#include "utils/Log.hpp"

struct Liquid
{
	enum class Type
	{
		None,
		Water,
		Blood,
		Oil,
		Count
	};
	Type type;
	double density;
	double viscosity;
	Color color;

	Liquid(Type type) : type(type)
	{
		switch (type)
		{
			case Type::Water:
				density = 1.0;
				viscosity = 0.01;
				color = Color(0, 50, 100);
				break;
			case Type::Blood:
				density = 1.0;
				viscosity = 0.05;
				color = Color(300, 0, 50);
				break;
			case Type::Oil:
				density = 0.9;
				viscosity = 0.005;
				color = Color(300, 200, 100);
				break;
			default:
				density = 0.0;
				viscosity = 0.0;
				color = Color{};
				break;
		}
	}


	static constexpr std::pair<const char*, Type> table[] = {
		{"water", Type::Water},
		{"blood", Type::Blood},
		{"oil",   Type::Oil}
	};

	static Type from_string(const std::string& str)
	{
		for (auto& [name, type] : table)
			if (str == name)
				return type;

		Log::error("Unknown liquid type: " + str);
		return Type::None;
	}

	static std::string to_string(Type type)
	{
		for (auto& [name, t] : table)
			if (t == type)
				return name;

		return "none";
	}

};
