#include "utils/Debug.hpp"
#include "external/entt/entt.hpp"
#include "components/Components.hpp"
#include "database/EntityFactory.hpp"
#include "nlohmann/json.hpp"

namespace Debug
{
	std::string entity_details(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Name>(entity))
			return "non-named entity";

		const auto name = registry.get<Name>(entity).name;
		const auto LUT = EntityFactory::instance().get_LUT();
		return LUT.at(name).dump(4);
	}
};
