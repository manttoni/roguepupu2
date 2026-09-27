#pragma once

#include <string>
#include <vector>

#include "game/Simulation.hpp"
#include "game/events/Event.hpp"

namespace Game::Event
{
	class Describer
	{
		public:
			static std::vector<std::string> describe(
					const Simulation& simulation,
					const Node& root);

		private:
			static std::string make_string(const Simulation& simulation, const Null& event);
			static std::string make_string(const Simulation& simulation, const Move& event);
			static std::string make_string(const Simulation& simulation, const Bump& event);
			static std::string make_string(const Simulation& simulation, const Attack& event);
			static std::string make_string(const Simulation& simulation, const AttackHit& event);
			static std::string make_string(const Simulation& simulation, const AttackMiss& event);
			static std::string make_string(const Simulation& simulation, const TakeDamage& event);
			static std::string make_string(const Simulation& simulation, const Spawn& event);
			static std::string make_string(const Simulation& simulation, const Destroy& event);
			static std::string make_string(const Simulation& simulation, const Drop& event);
			static std::string make_string(const Simulation& simulation, const Take& event);
			static std::string make_string(const Simulation& simulation, const Equip& event);
			static std::string make_string(const Simulation& simulation, const Unequip& event);
			static std::string make_string(const Simulation& simulation, const DiceRoll& event);
			static std::string make_string(const Simulation& simulation, const Death& event);
			static std::string make_string(const Simulation& simulation, const BecomeHostile& event);
			static std::string make_string(const Simulation& simulation, const ReceiveItem& event);
	};
}
