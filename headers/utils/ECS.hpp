#pragma once

#include <optional>
#include "components/Components.hpp"
#include "database/AbilityDatabase.hpp"
#include "database/LootTableDatabase.hpp"
#include "domain/Attribute.hpp"
#include "domain/Cave.hpp"
#include "domain/Color.hpp"
#include "domain/Event.hpp"
#include "domain/EventQueue.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/DevTools.hpp"
#include "infrastructure/GameLogger.hpp"
#include "infrastructure/GameSettings.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "systems/rendering/RenderData.hpp"
#include "systems/state/StateSystem.hpp"
#include "utils/Parser.hpp"
#include "utils/Random.hpp"
#include "utils/Utils.hpp"

namespace ECS
{
	template<typename... Components>
		std::vector<entt::entity> get_entities(const entt::registry& registry, const Components&... values)
		{
			std::vector<entt::entity> result;
			auto view = registry.view<std::decay_t<Components>...>();

			for (const auto entity : view)
			{
				bool match = ((registry.get<std::decay_t<Components>>(entity) == values) && ...);
				if (match)
					result.push_back(entity);
			}

			return result;
		}
	template<typename... Components>
		std::vector<entt::entity> get_entities_in_cave(const entt::registry& registry, const size_t cave_idx, const Components&... values)
		{
			std::vector<entt::entity> result;
			auto view = registry.view<std::decay_t<Components>...>();

			for (const auto entity : view)
			{
				if (!registry.all_of<Position>(entity) || registry.get<Position>(entity).cave_idx != cave_idx)
					continue;
				bool match = ((registry.get<std::decay_t<Components>>(entity) == values) && ...);
				if (match)
					result.push_back(entity);
			}

			return result;
		}

	inline entt::entity get_player(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().player;
	}



	inline Color get_fgcolor(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<FGColor>(entity))
			return Color::white();
		return registry.get<FGColor>(entity).color;
	}

	inline std::string get_name(const entt::registry& registry, const entt::entity entity)
	{
		const std::string& name = Utils::capitalize(registry.get<Name>(entity).name);
		return name;
	}

	inline NcursesAttr get_ncurses_attr(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<NcursesAttr>(entity))
			return registry.get<NcursesAttr>(entity);
		return NcursesAttr(A_NORMAL);
	}

	inline std::string get_colored_name(const entt::registry& registry, const entt::entity entity)
	{
		const Color& fgcolor = get_fgcolor(registry, entity);
		const NcursesAttr& attr = get_ncurses_attr(registry, entity);
		std::string name = fgcolor.markup() + get_name(registry, entity) + "{reset}";
		if (attr != A_NORMAL)
		{
			name.insert(0, attr.markup());
			name += "[reset]";
		}
		return name;
	}

	inline std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity> entities)
	{
		std::vector<std::string> colored_names;
		for (const auto entity : entities)
			colored_names.push_back(get_colored_name(registry, entity));
		return colored_names;
	}

	inline void queue_event(entt::registry& registry, const Event& event)
	{
		registry.ctx().get<EventQueue>().queue.push_back(event);
	}

	inline World& get_world(entt::registry& registry)
	{
		return registry.ctx().get<World>();
	}

	inline Cave& get_cave(entt::registry& registry, const Position& position)
	{
		return get_world(registry).get_cave(position.cave_idx);
	}

	inline Cell& get_cell(entt::registry& registry, const Position& position)
	{
		return get_cave(registry, position).get_cell(position);
	}

	inline const World& get_world(const entt::registry& registry)
	{
		return registry.ctx().get<World>();
	}

	inline const Cave& get_cave(const entt::registry& registry, const Position& position)
	{
		return get_world(registry).get_cave(position.cave_idx);
	}

	inline const Cell& get_cell(const entt::registry& registry, const Position& position)
	{
		return get_cave(registry, position).get_cell(position);
	}

	inline const Cave& get_cave(const entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<World>().get_cave(cave_idx);
	}

	inline Cave& get_cave(entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<World>().get_cave(cave_idx);
	}

	inline Cave& get_active_cave(entt::registry& registry)
	{
		const auto player = get_player(registry);
		assert(player != entt::null);
		assert(registry.all_of<Position>(player));
		const auto& pos = registry.get<Position>(player);
		return get_cave(registry, pos);
	}

	inline const Cave& get_active_cave(const entt::registry& registry)
	{
		const auto player = get_player(registry);
		assert(player != entt::null);
		assert(registry.all_of<Position>(player));
		const auto& pos = registry.get<Position>(player);
		return get_cave(registry, pos);
	}

	inline entt::entity get_unlinked_passage(const entt::registry& registry, const size_t cave_idx)
	{
		const auto unlinked_passages = get_entities_in_cave(registry, cave_idx, Transition{.destination = entt::null});
		assert(!unlinked_passages.empty());
		return unlinked_passages[Random::rand<size_t>(0, unlinked_passages.size() - 1)];
	}

	inline size_t get_turn_number(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().turn_number;
	}

	inline void destroy_entity(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<Destroyed>(entity);

		Event destroy_event = {.type = Event::Type::Destroy};
		destroy_event.target.entity = entity;
		queue_event(registry, destroy_event);
	}

	inline double distance(const entt::registry& registry, const Position& a, const Position& b)
	{
		assert(a.cave_idx == b.cave_idx);
		return get_cave(registry, a).distance(a, b);
	}

	inline double distance(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		assert(registry.all_of<Position>(a) && registry.all_of<Position>(b));
		return distance(registry, registry.get<Position>(a), registry.get<Position>(b));
	}

	inline wchar_t get_glyph(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Glyph>(entity))
			return registry.get<Glyph>(entity).glyph;
		return registry.get<Name>(entity).name[0];
	}

	inline double get_liquid_level(const entt::registry& registry, const Position& pos)
	{
		double liquid_level = get_cell(registry, pos).get_liquid_level();
		for (const auto entity : get_entities(registry, pos))
		{
			if (registry.all_of<Size>(entity))
				liquid_level += registry.get<Size>(entity).liters;
		}
		return liquid_level;
	}

	inline size_t generate_cave(entt::registry& registry, const std::string& conf_id = "default")
	{
		const size_t cave_idx = get_world(registry).new_cave();
		CaveGenerator::Data data(registry, get_cave(registry, cave_idx));
		Parser::parse_cave_generation_conf(conf_id, data);
		CaveGenerator::generate_cave(data);
		return cave_idx;
	}

	inline void init_registry(entt::registry& registry)
	{
		registry.ctx().emplace<GameState>();
		registry.ctx().emplace<GameLogger>();
		registry.ctx().emplace<AbilityDatabase>();
		registry.ctx().emplace<World>();
		registry.ctx().emplace<RenderData>();
		registry.ctx().emplace<EventQueue>();
		registry.ctx().emplace<Dev>();
		registry.ctx().emplace<LootTableDatabase>();
		registry.ctx().emplace<GameSettings>();
	}

	/* Get attack range of creature or weapon.
	 * If creature, not all it's attacks/weapons have to be in that range,
	 * but create range from shortest and longest possible ranges from any weapon/attack,
	 * even unarmed attacks. Combat system will then decide what attacks will be used.
	 *
	 * This is for checking whether an entity can attack something based on just distance.
	 * */
	inline Range<double> get_attack_range(const entt::registry& registry, const entt::entity entity)
	{
		// Could happen if trying to get range of "unarmed weapon slot", but then give creature instead
		assert(entity != entt::null);

		const bool is_creature = registry.get<Category>(entity).category == "creatures";
		const bool is_weapon = registry.get<Subcategory>(entity).subcategory == "weapons";
		// const bool is_shield... this will be here, but no shields yet

		if (is_weapon)
			return registry.get<AttackRange>(entity).range;
		else if (is_creature)
		{
			Range<double> range = registry.get<AttackRange>(entity).range; // This is unarmed
			if (registry.all_of<EquipmentSlots>(entity))
			{
				const auto& equipped = registry.get<EquipmentSlots>(entity).equipped_items;
				const auto main_hand = equipped.at(Equipment::Slot::MainHand);
				const auto off_hand = equipped.at(Equipment::Slot::OffHand);
				if (main_hand != entt::null)
				{
					const auto& main_hand_range = get_attack_range(registry, main_hand);
					range.min = std::min(range.min, main_hand_range.min);
					range.max = std::max(range.max, main_hand_range.max);
				}
				if (off_hand != entt::null && off_hand != main_hand)
				{
					const auto& off_hand_range = get_attack_range(registry, off_hand);
					range.min = std::min(range.min, off_hand_range.min);
					range.max = std::max(range.max, off_hand_range.max);
				}
			}
			return range;
		}
		std::abort(); // this function is still unfinished, needs shield bash ranges etc
	}

	inline Attributes get_attributes(const entt::registry& registry, const entt::entity entity)
	{
		using namespace StateSystem;
		Attributes attributes = {
			.strength = get_attribute<Strength>(registry, entity),
			.dexterity = get_attribute<Dexterity>(registry, entity),
			.agility = get_attribute<Agility>(registry, entity),
			.perception = get_attribute<Perception>(registry, entity),
			.vitality = get_attribute<Vitality>(registry, entity),
			.endurance = get_attribute<Endurance>(registry, entity),
			.willpower = get_attribute<Willpower>(registry, entity),
			.charisma = get_attribute<Charisma>(registry, entity)
		};
		return attributes;
	}

	inline void spawn_liquid(entt::registry& registry, const Position& position, const LiquidMixture& lm)
	{
		auto& cell = get_cell(registry, position);
		cell.get_liquid_mixture() += lm;
	}
};
