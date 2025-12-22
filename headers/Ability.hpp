#pragma once

#include <string>
#include "Cell.hpp"
#include "entt.hpp"

class Ability
{
	private:
		std::string id; // is also the name
		std::string category; // spells or innate
									 // spells can be learned
									 // innate can only be born with
		std::string target; // self
		size_t cooldown;
		std::string summon; // id/name of summoned entity if any
		size_t use_turn; // 0 if never used, turn numbers start at 1

	public:
		Ability() = default;
		Ability(const Ability& other) = default;
		Ability& operator=(const Ability& other) = default;
		std::string get_id() const { return id; }
		std::string get_target() const { return target; }
		Ability(const std::string& id, const std::string& category, const std::string& target, const size_t cooldown, const std::string& summon);
		void use(entt::registry& registry, Cell* target);
		bool on_cooldown(const entt::registry& registry) const;
};
