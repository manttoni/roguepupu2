#include <vector>
#include <memory>
#include <string>
#include "Color.hpp"
#include "Effect.hpp"
#include "Light.hpp"
#include "EntityFactory.hpp"
#include "Fungus.hpp"
#include "Creature.hpp"

std::unique_ptr<Fungus> EntityFactory::get_fungus(const std::string& name) const
{
	static const std::vector<Fungus> fungi =
	{
		{
			"glowing",
			L'*',
			Color(0, 0, 500),
			{ Light(5, SIZE_MAX, Color(5, 0, 50)) }
		},
		{
			"woody",
			L'$',
			Color(666, 333, 0),
			{}
		}
	};

	for (const auto& f : fungi)
		if (f.get_name() == name)
			return std::make_unique<Fungus>(f);
	throw std::runtime_error("Fungus " + name + " doesn't exist");
}

std::unique_ptr<Creature> EntityFactory::get_creature(const std::string& name) const
{
	static const std::vector<Creature> creatures =
	{
		{
			"Rabdin",
			L'𖨆',
			Color(123,456,789),
			10
		}
	};

	for (const auto& c : creatures)
		if (c.get_name() == name)
			return std::make_unique<Creature>(c);
	throw std::runtime_error("Creature " + name + " doesn't exist");
}
