#include <stddef.h>
#include <stdint.h>
#include <nlohmann/detail/json_ref.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <optional>
#include <utility>

#include "UI/Dialog.hpp"
#include "database/EntityFactory.hpp"
#include "infrastructure/DevTools.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "components/Components.hpp"
#include "domain/Liquid.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"

namespace DevTools
{



	void dev_menu(entt::registry& registry)
	{
		(void) registry;
	}
};
