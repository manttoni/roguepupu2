#pragma once

#include "external/entt/fwd.hpp"
#include <optional>
#include "Position.hpp"

struct Event
{
	enum class Type
	{
		None,
		Move,
		Gather,
	};

	Type type = Type::None;
	std::optional<entt::entity> gatherable;
	std::optional<Position> move_from, move_to;
};
