#include "application/EventDescriber.hpp"
#include "application/EntityDescriber.hpp"
#include "game/events/Event.hpp"
#include "game/Simulation.hpp"
#include "utils/Log.hpp"

namespace EventDescriber
{
	namespace Event = Game::Event;

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Attack& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Attack: attacker=" << ed.describe(event.attacker)
			<< ", defender=" << ed.describe(event.defender)
			<< ", weapon=" << ed.describe(event.weapon)
			<< ", ammunition=" << ed.describe(event.ammunition)
			<< ", advantage=" << event.advantage;

		std::string desc =
			ed.describe(event.attacker) + " attacks " +
			ed.describe(event.defender);

		if (event.weapon != entt::null)
			desc += " with " + ed.describe(event.weapon);

		return desc + ".";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::AttackHit& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "AttackHit: attacker=" << ed.describe(event.attacker)
			<< ", defender=" << ed.describe(event.defender)
			<< ", weapon=" << ed.describe(event.weapon)
			<< ", ammunition=" << ed.describe(event.ammo)
			<< ", advantage=" << event.advantage;

		return ed.describe(event.attacker) + " hits " +
			ed.describe(event.defender) + ".";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::AttackMiss& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "AttackMiss: attacker=" << ed.describe(event.attacker)
			<< ", defender=" << ed.describe(event.defender)
			<< ", weapon=" << ed.describe(event.weapon)
			<< ", ammunition=" << ed.describe(event.ammo)
			<< ", advantage=" << event.advantage;

		return ed.describe(event.attacker) + " misses " +
			ed.describe(event.defender) + ".";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::TakeDamage& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "TakeDamage: target=" << ed.describe(event.target)
			<< ", source=" << ed.describe(event.source)
			<< ", damage_type=" << event.damage_type
			<< ", amount=" << event.amount;

		std::ostringstream desc;
		desc << ed.describe(event.target)
			<< " takes " << event.amount << " damage.";

		return desc.str();
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Spawn& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Spawn: entity=" << ed.describe(event.entity)
			<< ", position=" << event.position;

		std::ostringstream desc;
		desc << ed.describe(event.entity)
			<< " appears at " << event.position << ".";

		return desc.str();
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Destroy& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Destroy: entity=" << ed.describe(event.entity);

		return ed.describe(event.entity) + " is destroyed.";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Drop& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Drop: entity=" << ed.describe(event.entity)
			<< ", item=" << ed.describe(event.item)
			<< ", position=" << event.position;

		return ed.describe(event.entity) + " drops " +
			ed.describe(event.item) + ".";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Take& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Take: entity=" << ed.describe(event.entity)
			<< ", item=" << ed.describe(event.item)
			<< ", position=" << event.position;

		return ed.describe(event.entity) + " takes " +
			ed.describe(event.item) + ".";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Equip& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Equip: entity=" << ed.describe(event.entity)
			<< ", equipment=" << ed.describe(event.equipment);

		return ed.describe(event.entity) + " equips " +
			ed.describe(event.equipment) + ".";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Unequip& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Unequip: entity=" << ed.describe(event.entity)
			<< ", equipment=" << ed.describe(event.equipment);

		return ed.describe(event.entity) + " unequips " +
			ed.describe(event.equipment) + ".";
	}

	std::string describe(
			const Game::Simulation&,
			const Event::DiceRoll& event)
	{
		Log::debug()
			<< "DiceRoll: dice=" << event.dice
			<< ", result=" << event.result
			<< ", difficulty=" << event.difficulty;

		std::ostringstream desc;
		desc << "Roll: " << event.dice
			<< " = " << event.result
			<< " against " << event.difficulty << ".";

		return desc.str();
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::Death& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "Death: entity=" << ed.describe(event.entity);

		return ed.describe(event.entity) + " dies.";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::BecomeHostile& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "BecomeHostile: entity=" << ed.describe(event.entity)
			<< ", target=" << ed.describe(event.target);

		return ed.describe(event.entity) +
			" becomes hostile toward " +
			ed.describe(event.target) + ".";
	}

	std::string describe(
			const Game::Simulation& simulation,
			const Event::ReceiveItem& event)
	{
		EntityDescriber ed(simulation.get_registry());

		Log::debug()
			<< "ReceiveItem: entity=" << ed.describe(event.entity)
			<< ", item=" << ed.describe(event.item);

		return ed.describe(event.entity) + " receives " +
			ed.describe(event.item) + ".";
	}
	/* Meant for converting an event tree into a vector of strings.
	 * Those strings will be displayed to the player
	 * */
	std::vector<std::string> describe(
			const Game::Simulation& simulation,
			const Event::Node& root)
	{
		assert(root.outcome.has_value() && "root event not simulated");

		if (root.outcome.value() == Event::Outcome::Rejected)
		{
			assert(
					root.consequences.empty() &&
					"rejected node has consequences");
			return {};
		}

		std::vector<std::string> descriptions;

		std::visit(
				[&](const auto& event)
				{
				if constexpr (requires { describe(simulation, event); })
				descriptions.push_back(describe(simulation, event));
				},
				root.event);

		for (const auto& consequence : root.consequences)
		{
			auto children = describe(simulation, consequence);
			descriptions.insert(
					descriptions.end(),
					std::make_move_iterator(children.begin()),
					std::make_move_iterator(children.end()));
		}

		return descriptions;
	}
}
