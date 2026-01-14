#pragma once

#include "domain/Liquid.hpp"
#include "external/entt/fwd.hpp"

struct Dev
{
	bool god_mode = false;
	bool show_debug = false;
	Liquid::Type liquid_type = Liquid::Type::Water;
};

namespace DevTools
{
	void dev_menu(entt::registry& registry);
};
