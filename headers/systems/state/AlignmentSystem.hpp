#pragma once

#include "external/entt/fwd.hpp"

namespace AlignmentSystem
{
	double get_opinion(const entt::registry& registry, const entt::entity a, const entt::entity b);
	void lose_opinion(entt::registry& registry, const entt::entity a, const entt::entity b, double amount);
	bool is_hostile(const entt::registry& registry, const entt::entity a, const entt::entity b);
	bool is_friendly(const entt::registry& registry, const entt::entity a, const entt::entity b);
	bool is_neutral(const entt::registry& registry, const entt::entity a, const entt::entity b);
};
