#pragma once

#include "Color.hpp"
#include "ColorPair.hpp"

struct Liquid
{
	enum class Type
	{
		None,
		Water,
		Blood,
		Oil,
	};
	Type type;
	double density;
	Color color;

	Liquid(Type type) : type(type)
	{
		switch (type)
		{
			case Type::Water:
				density = 1.0;
				color = Color(0, 100, 300);
				break;
			case Type::Blood:
				density = 1.0;
				color = Color(300, 0, 100);
				break;
			case Type::Oil:
				density = 0.9;
				color = Color(300, 200, 100);
				break;
			default:
				density = 0.0;
				color = Color{};
				break;
		}
	}
};

class LiquidMixture
{
	private:
		std::map<Liquid::Type, double> mixture;

	public:
		LiquidMixture() = default;
		LiquidMixture(const LiquidMixture& other) = default;
		LiquidMixture& operator=(const LiquidMixture& other) = default;

		LiquidMixture(const Liquid::Type type, const double volume);
		LiquidMixture& operator+=(const LiquidMixture& other);

		LiquidMixture flow(const double volume);
		double remove_liquid(const Liquid::Type type, const double volume);
		void add_liquid(const Liquid::Type type, const double volume);
		double get_volume() const;
		ColorPair get_color_pair() const;
		Color get_fgcolor() const;
		Color get_bgcolor() const;
		wchar_t get_glyph() const;
};
