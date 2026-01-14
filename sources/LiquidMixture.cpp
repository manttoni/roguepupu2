#include <cassert>
#include "Unicode.hpp"
#include "LiquidMixture.hpp"
#include "ColorPair.hpp"
#include "Color.hpp"

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
	assert(volume >= 0);
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

/* Rendering stuff
 * ColorPair tells most common liquid as well as least dense
 * bgcolor = biggest volume, fgcolor = least dense (floats on top) */
ColorPair LiquidMixture::get_color_pair() const
{
	return ColorPair(get_fgcolor(), get_bgcolor());
}

Color LiquidMixture::get_fgcolor() const
{
	auto largest_volume = Liquid::Type::None;
	for (const auto& [type, volume] : mixture)
	{
		if (largest_volume == Liquid::Type::None || mixture.at(type) > mixture.at(largest_volume))
			largest_volume = type;
	}
	return Liquid(largest_volume).color;
}

Color LiquidMixture::get_bgcolor() const
{
	return get_fgcolor() / 10;
}

/* Glyph tells how deep the liquid is */
wchar_t LiquidMixture::get_glyph() const
{
	const double volume = get_volume();
	if (volume >= 0.5)
		return Unicode::LiquidDeep;
	if (volume >= 0.1)
		return Unicode::LiquidMedium;
	return Unicode::LiquidShallow;
}
