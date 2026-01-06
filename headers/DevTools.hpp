#pragma once

#include "entt.hpp"

struct Dev
{
	bool god_mode = false;
	bool show_debug = false;
};

namespace DevTools
{
	void dev_menu(entt::registry& registry);
};
