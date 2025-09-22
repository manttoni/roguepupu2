#include "Effect.hpp"
#include "Cave.hpp"

Effect::Effect() {}

Effect::Effect(
		const Type type,
		const short color_id,
		const double radius) :
	type(type), color_id(color_id), radius(radius)
{}

void Effect::trigger(Cave& cave, const size_t idx)
{
	auto affected = cave.get_nearby_ids(idx, radius);
	affected.push_back(idx);
	auto& cells = cave.get_cells();
	switch (type)
	{
		case Type::GLOW:
			for (size_t i = 0; i < affected.size(); ++i)
			{
				if (!cave.has_vision(idx, affected[i]))
					continue;
				cells[affected[i]].add_light(color_id);
			}
			break;
		case Type::NONE:
			break;
	}
}
