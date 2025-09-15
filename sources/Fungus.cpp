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
				"Glowing mushroom",
				UI::instance().GLOWING_FUNGUS,
				'*',
				Effect(Effect::Type::GLOW, UI::instance().LIGHT_BLUE, 5)
			}
		}
	};
	return LUT[type];
}

Fungus::Fungus() :
	Entity("default", 0),
	type(Type::NONE)
{}
Fungus::Fungus(const Type type) :
	Entity(	get_info(type).name,
			get_info(type).color_pair_id),
	type(type)
{
	switch (type)
	{
		case Type::GLOWING:
			Entity::set_char(get_info(type).ch);
			Entity::add_effect(get_info(type).effect);
			break;
		default:
			break;
	}
}

