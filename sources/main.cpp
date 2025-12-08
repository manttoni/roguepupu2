#include <string>             // for allocator, basic_string
#include "EntityFactory.hpp"  // for EntityFactory
#include "Menu.hpp"           // for Menu
#include "UI.hpp"             // for UI
#include "Utils.hpp"          // for log
#include "Tester.hpp"

void run()
{
	UI::instance().get_menu("main").loop();
}

int main(void)
{
	Log::log("--- Run started ---");
	EntityFactory::instance().init();
	if (tester() > 0)
		return 1;
	Log::log("EntityFactory initialized");
	UI::instance().init();
	Log::log("UI initialized");
	run();
	Log::log("Run completed");
	UI::instance().end();
}

