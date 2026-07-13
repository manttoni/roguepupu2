#include <sstream>
#include "utils/Debug.hpp"
#include "utils/ECS.hpp"
#include "external/entt/entt.hpp"
#include "components/Components.hpp"
#include "database/EntityFactory.hpp"
#include "nlohmann/json.hpp"
#include "systems/environment/LiquidSystem.hpp"

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

	/* Return a text vector that will be displayed on the screen during debugging
	 * */
	std::vector<std::string> debug_text(const entt::registry& registry)
	{
		std::vector<std::string> text;
		std::stringstream ss;

		const auto player = ECS::get_player(registry);
		const auto& player_position = registry.get<Position>(player);
		const auto& cave = ECS::get_cave(registry, player_position);
		const auto& player_vector = Vec2<int>::from_idx(player_position.cell_idx, cave.get_size());
		ss << "Player position: " << player_position << std::endl;
		ss << "Player vector: " << player_vector << std::endl;
		ss << "Cave size: " << cave.get_size() << std::endl;
		ss << "Cell amount: " << cave.get_cells().size() << std::endl;

		const auto& entities = *registry.storage<entt::entity>();

		ss << "Number of Entities: " << entities.size() << std::endl;

		ss << "Liquids volume: " << LiquidSystem::get_liquids_volume(registry, cave.get_idx()) << std::endl;

		std::string line;
		while (std::getline(ss, line))
			text.push_back(line);
		return text;
	}
};
