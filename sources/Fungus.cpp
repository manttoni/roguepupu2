#include <string>
#include <map>
#include "Fungus.hpp"
#include "UI.hpp"

Fungus::Info Fungus::get_info(const Fungus::Type type)
{
	static std::map<Fungus::Type, Fungus::Info> LUT =
	{
		{
			Fungus::Type::NONE,
			{
				"default",
				Color(0, 0, 0),
				'?',
				Light()
			}
		},
		{
			Fungus::Type::GLOWING,
			{
				"Glowing fungus",
				Color(0, 0, 500),
				'*',
				Light(Color(0, 0, 50), 5)
			}
		},
		{
			Fungus::Type::WOODY,
			{
				"Wooden fungus",
				Color(1000, 666, 0),
				'$',
				Light()
			}
		}
	};
	return LUT[type];
}

Fungus::Fungus() {}
Fungus::Fungus(const Type type, Cell* cell) :
	Entity(get_info(type).name, get_info(type).color, get_info(type).ch, cell),
	type(type)
{
	switch (type)
	{
		case Type::GLOWING:
			Entity::add_light(get_info(type).light);
			break;
		case Type::WOODY:
			break;
		default:
			break;
	}
}

