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
	idx(idx),
	glyph(L'\0')
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
}

/* Return a default Cell glyph by Type.
 * Can give custom value and will return that.
 * Custom values are at least useful for DevTools,
 * like showing elevation values or liquid level etc...
 * */
wchar_t Cell::get_glyph() const
{
	if (glyph != L'\0')
		return glyph;

	switch (get_type())
	{
		case Type::Rock:
			return Unicode::FullBlock;
		case Type::Floor:
			{
				const std::string floor_chars = ",.:;\'\"";
				return floor_chars[Random::randsize_t(0, floor_chars.size() - 1, idx)];
			}
			break;
		case Type::Source:
			return Unicode::Triangle;
		case Type::Sink:
			return Unicode::InvertedTriangle;
		default:
			Error::fatal("Unknown cell type");
	}
}

void Cell::reduce_density(const double amount)
{
	density -= amount;
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
