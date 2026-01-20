#include "external/entt/entt.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "components/Components.hpp"

namespace AlignmentSystem
{
	/* Get a's opinion of b.
	 * Certain actions will affect opinions.
	 * Symbols can affect alignment?
	 * */
	double get_opinion(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		assert(registry.all_of<Alignment>(a) && registry.all_of<Alignment>(b));
		const auto& aA = registry.get<Alignment>(a);
		const auto& bA = registry.get<Alignment>(b);
		double opinion = aA.tolerance - aA.distance(bA);
		if (registry.all_of<Charisma>(b))
			opinion += StateSystem::get_attribute<Charisma>(registry, b) / 10;

		return opinion;
	}

	/* Is a hostile towards b?
	 * Will attack etc if is
	 * */
	bool is_hostile(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		return get_opinion(registry, a, b) <= -1; // Save these magic numbers somewhere
	}

	/* Friendlies will help each other
	 * */
	bool is_friendly(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		return get_opinion(registry, a, b) >= 1;
	}

	bool is_neutral(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		return !is_hostile(registry, a, b) && !is_friendly(registry, a, b);
	}
};
