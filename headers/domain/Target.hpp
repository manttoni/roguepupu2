#pragme once

#include <optional>
#include "external/entt/fwd.hpp"
#include "Position.hpp"

struct Target
{
	enum class Type
	{
		None,
		Self,
		Actor,
		Cell,
		Entity,
		Enemy,
		Ally,
	};

	Type type = Type::None;
	std::optional<entt::entity> entity;
	std::optional<Position> position;
	double range = 0.0;
};
