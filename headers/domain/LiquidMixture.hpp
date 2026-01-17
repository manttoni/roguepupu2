#pragma once

#define TRESHOLD_LIQUID_DEEP 1.0
#define TRESHOLD_LIQUID_MEDIUM 0.5
#define TRESHOLD_LIQUID_SHALLOW 0.0

#include <map>
#include "domain/Color.hpp"
#include "domain/Liquid.hpp"

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
		double get_volume(const Liquid::Type type = Liquid::Type::None) const;
		double get_viscosity() const;
		Color get_color() const;
};
