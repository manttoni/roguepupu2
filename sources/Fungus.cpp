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
				UI::instance().DEFAULT,
				'?',
				Effect()
			}
		},
		{
			Fungus::Type::GLOWING,
			{
				"Glowing fungus",
				UI::instance().GLOWING_FUNGUS,
				'*',
				Effect(Effect::Type::GLOW, UI::instance().LIGHT_BLUE, 5)
			}
		},
		{
			Fungus::Type::WOODY,
			{
				"Wooden fungus",
				UI::instance().WOODY_FUNGUS,
				'$',
				Effect()
			}
		}
	};
	return LUT[type];
}

Fungus::Fungus() {}
Fungus::Fungus(const Type type, Cell* cell) :
	Entity(get_info(type).name, get_info(type).color_pair_id, get_info(type).ch, cell),
	type(type)
{
	switch (type)
	{
		case Type::GLOWING:
			Entity::add_effect(get_info(type).effect);
			break;
		case Type::WOODY:
			break;
		default:
			break;
	}
}

