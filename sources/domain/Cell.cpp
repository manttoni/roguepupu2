#include <stddef.h>
#include <format>
#include <string>

#include "domain/Cell.hpp"        // for Cell
#include "utils/Error.hpp"
#include "domain/LiquidMixture.hpp"
#include "domain/Color.hpp"
#include "utils/Unicode.hpp"
#include "utils/Random.hpp"

Cell::Cell(const size_t idx, const Cell::Type type) :
	idx(idx)
{
	switch (type)
	{
		case Cell::Type::Rock:
			density = CELL_DENSITY_MAX;
			break;
		case Cell::Type::Floor:
			density = 0;
			break;
		default:
			Error::fatal("Don't use other types here");
	}
	set_glyph();
}

wchar_t Cell::get_glyph() const
{
	return glyph;
}

void Cell::set_glyph()
{
	switch (get_type())
	{
		case Type::Rock:
			glyph = Unicode::FullBlock;
			break;
		case Type::Floor:
			{
				const std::string floor_chars = ",.:;\'\"";
				glyph = floor_chars[Random::rand<size_t>(0, floor_chars.size() - 1)];
			}
			break;
		case Type::Source:
			glyph = Unicode::Triangle;
			break;
		case Type::Sink:
			glyph = Unicode::InvertedTriangle;
			break;
		default:
			Error::fatal("Unknown cell type");
	}
}

void Cell::reduce_density(const double amount)
{
	density -= amount;
	set_glyph();
}

double Cell::get_liquid_level() const
{
	return density + liquid_mixture.get_volume();
}

std::string Cell::to_string() const
{
	std::string string = "";
	switch (get_type())
	{
		case Type::Rock:
			string += "Rock | Density: " + std::format("{:.2f}", density);
			break;
		case Type::Floor:
			string += "Floor | Elevation: " + std::format("{:.2f}", density);
			break;
		case Type::Source:
			string += "Source";
			break;
		case Type::Sink:
			string += "Sink";
			break;
		default:
			break;
	}
	return string;
}
