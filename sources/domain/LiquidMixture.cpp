#include <cassert>
#include <algorithm>
#include <format>
#include <map>
#include <string>
#include <utility>

#include "domain/LiquidMixture.hpp"
#include "domain/Color.hpp"
#include "domain/Liquid.hpp"

LiquidMixture::LiquidMixture(const Liquid::Type type, const double volume)
{
	mixture[type] = volume;
}

LiquidMixture& LiquidMixture::operator+=(const LiquidMixture& other)
{
	for (const auto& [type, volume] : other.mixture)
		mixture[type] += volume;
	return *this;
}

/* Take some of every type of liquid in the mixture */
LiquidMixture LiquidMixture::flow(const double flow_volume)
{
	LiquidMixture mix;
	double this_volume;
	while (mix.get_volume() < flow_volume &&
		(this_volume = get_volume()) > 0)
	{
		for (const auto& [type, volume] : mixture)
		{
			// percentage of this type in the whole mixture
			const double part = volume / this_volume;
			assert(part <= 1 && part >= 0);
			const double removed = remove_liquid(type, flow_volume * part);
			mix.add_liquid(type, removed);
		}
	}
	return mix;
}

double LiquidMixture::remove_liquid(const Liquid::Type type, const double volume)
{
	if (volume < 0)
		Error::fatal("Volume is lt 0: " + std::to_string(volume));
	const double remove = std::min(mixture[type], volume);
	mixture[type] -= remove;
	assert(mixture[type] >= 0);
	return remove;
}

void LiquidMixture::add_liquid(const Liquid::Type type, const double volume)
{
	assert(volume >= 0);
	mixture[type] += volume;
}

double LiquidMixture::get_volume(const Liquid::Type type) const
{
	double volume_total = 0;
	for (const auto& [liquid_type, liquid_volume] : mixture)
	{
		if (type != Liquid::Type::None && type != liquid_type)
			continue;
		volume_total += liquid_volume;
	}
	return volume_total;
}

double LiquidMixture::get_viscosity() const
{
	double viscosity = 0;
	const double this_volume = get_volume();
	for (const auto& [type, volume] : mixture)
	{
		const double part = volume / this_volume;
		viscosity += part * Liquid(type).viscosity;
	}
	return viscosity;
}

Color LiquidMixture::get_color() const
{
	auto largest_volume = Liquid::Type::None;
	for (const auto& [type, volume] : mixture)
	{
		if (largest_volume == Liquid::Type::None || mixture.at(type) > mixture.at(largest_volume))
			largest_volume = type;
	}
	return Liquid(largest_volume).color;
}

std::string LiquidMixture::to_string() const
{
	std::string string = "";
	for (auto it = mixture.begin(); it != mixture.end(); ++it)
	{
		if (get_volume() == 0.0)
			break;
		const auto type = it->first;
		if (it != mixture.begin())
			string += " | ";
		string += Liquid(type).color.markup();
		string += Liquid::to_string(type);
		string += "(" + std::format("{:.1f}", 100.0 * it->second / get_volume()) + "%)";
		string += "{reset}";
	}
	return string;
}
