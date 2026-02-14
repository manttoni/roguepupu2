#include <assert.h>
#include <vector>

#include "components/Components.hpp"
#include "domain/Event.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "utils/ECS.hpp"

namespace AlignmentSystem
{
	/* Get a's opinion of b.
	 * Certain actions will affect opinions.
	 * Symbols can affect alignment?
	 * */
	double get_opinion(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		assert(registry.all_of<Alignment>(a));
		assert(registry.all_of<Alignment>(b));
		const auto& aA = registry.get<Alignment>(a);
		const auto& bA = registry.get<Alignment>(b);
		double opinion = aA.tolerance - aA.distance(bA);
		opinion += hypot(2, 2) / 2; // if alignment in center, everything else is neutral. Same alignment will be > 0. Should work
		if (registry.all_of<Charisma>(b))
			opinion += StateSystem::get_attribute<Charisma>(registry, b) / 10;

		// if a has personal opinions of b
		if (aA.personal_opinions.contains(b))
			opinion += aA.personal_opinions.at(b);

		return opinion;
	}

	/* a loses opinion of b
	 * */
	void lose_opinion(entt::registry& registry, const entt::entity a, const entt::entity b, const double amount)
	{
		if (registry.all_of<Dead>(a) || !registry.all_of<Alignment>(a))
			return;
		const bool hostility_before = is_hostile(registry, a, b);
		auto& aA = registry.get<Alignment>(a);
		aA.personal_opinions[b] -= amount;
		const bool hostility_after = is_hostile(registry, a, b);

		if (hostility_before == hostility_after)
			return;

		Event event;
		event.type = Event::Type::BecomeHostile;
		event.actor.entity = a;
		event.target.entity = b;
		ECS::queue_event(registry, event);
	}

	/* Is a hostile towards b?
	 * Will attack etc if is
	 * */
	bool is_hostile(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		return get_opinion(registry, a, b) < 0.0; // Save these magic numbers somewhere
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
