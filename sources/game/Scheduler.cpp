#include "game/Scheduler.hpp"
#include "utils/Log.hpp"
#include <algorithm>
#include <iterator>
namespace Game::Turn
{
	void Scheduler::next_turn()
	{
		current++;
		if (current == actors.size())
		{
			current = 0;
			round++;
			Log::info() << "Round " << round << " begins";
		}
	}

	void Scheduler::add(Actor actor)
	{
		const auto insertion = std::upper_bound(
				actors.begin(),
				actors.end(),
				actor.initiative,
				[](int initiative, const Actor& existing)
				{
					return initiative > existing.initiative;
				});

		const auto index = static_cast<std::size_t>(
				std::distance(actors.begin(), insertion));

		const bool shifts_current =
			!actors.empty() && index <= current;

		actors.insert(insertion, actor);

		if (shifts_current)
			++current;
	}
}
