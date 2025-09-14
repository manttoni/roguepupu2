#include "Effect.hpp"

Effect::Effect() {}

Effect::Effect(
		const Type type,
		const Color& color,
		const size_t duration,
		const double radius) :
	type(type), color(color), duration(duration), radius(radius)
{}

void Effect::trigger(Cave& cave, const size_t idx)
{
	auto affected = cave.get_nearby_ids(idx, radius);
	affected.push_back(idx);
	std::vector<Cell>& cells = cave.get_cells();
	switch (type)
	{
		case Type::GLOW:
			for (size_t i = 0; i < affected.size(); ++i)
			{
				cells[i].add_glow(color);
			}
			break;
		case Type::NONE:
			break;
	}
}
