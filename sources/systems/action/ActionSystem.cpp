#include "UI/UI.hpp"
#include "components/Components.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Event.hpp"
#include "domain/World.hpp"
#include "entt.hpp"
#include "infrastructureGameState.hpp"
#include "systems/action/AISystem.hpp"
#include "systems/action/AbilitySystem.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/combat/CombatSystem.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/position/PositionSystem.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "systems/rendering/RenderingSystem.hpp"
#include "systems/state/ContextSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "testing/DevTools.hpp"
#include "utils/ECS.hpp"

namespace ActionSystem
{
	// At this point the intent has been accepted and will be acted/executed
	void resolve_intent(entt::registry& registry, Intent intent)
	{
		switch (intent.type)
		{
			case Intent::Type::ExamineCell:
				ContextSystem::examine_cell(registry, intent.target.position);
				break;
			case Intent::Type::OpenInventory:
				ContextSystem::show_entities_list(registry, intent.target.entity);
				break;
			case Intent::Type::ShowPlayer:
				ContextSystem::show_entity_details(registry, intent.actor);
				break;
			case Intent::Type::Move:
				MovementSystem::move(registry, intent.actor, intent.target.position);
				break;
			case Intent::Type::Attack:
				CombatSystem::attack(registry, intent.actor, intent.target.entity);
				break;
			case Intent::Type::UseAbility:
				AbilitySystem::use_ability(registry, intent.actor, intent.ability_id, intent.target);
				break;
			case Intent::Type::Gather:
				GatheringSystem::gather(registry, intent.actor, intent.target.entity);
				break;
			case Intent::Type::Hide:
				if (registry.all_of<Hidden>(intent.actor))
					registry.erase<Hidden>(intent.actor);
				else
					registry.emplace<Hidden>(intent.actor);
				break;
			case Intent::Type::DoNothing:
			case Intent::Type::None:
			default:
				return;
		}
		RenderingSystem::render(registry);
	}

	Intent get_direction_intent(const entt::registry& registry, const entt::entity actor, const Vec2 direction)
	{
		const auto& current_pos = registry.get<Position>(actor);
		const auto& cave = PositionSystem::get_cave(registry, current_pos);
		const auto cave_size = cave.get_size();

		const Vec2 current(current_pos.cell_idx, cave_size);
		const Vec2 dst = current + direction;

		if (dst.out_of_bounds(0, cave_size() - 1))
			Log::error("Destination position out of bounds");

		const size_t dst_idx = dst.to_idx(cave_size);
		const Position dst_pos{ dst_idx, cave.get_idx() };
		if (MovementSystem::can_move(registry, current_pos, dst_pos))
		{
			Intent intent = {.type = Intent::Type::Move};
			intent.target.position = dst_pos;
			return intent;
		}
		return {.type = Intent::Type::None};
	}

	Intent get_player_intent(entt::registry& registry)
	{
		const auto player = ECS::get_player(registry);
		while (true)
		{
			RenderingSystem::render(registry);

			int key = UI::instance().input(500);
			if (key == '`') DevTools::dev_menu(registry);
			const Vec2 direction = UI::instance().get_direction(key);
			if (direction != Vec2{0, 0})
				return get_direction_intent(registry, player, direction);
			switch (key)
			{
				case KEY_RIGHT_CLICK:
					{
					Intent intent = {.type = Intent::Type::Attack};
					intent.target.position = UI::instance().get_clicked_cell(registry);
					return intent;
					}
				case KEY_LEFT_CLICK:
					{
					Intent intent = {.type = Intent::Type::ExamineCell};
					intent.target.position = UI::instance().get_clicked_cell(registry);
					return intent;
					}
				case 'i':
					{
					Intent intent = {.type = Intent::Type::OpenInventory};
					intent.target.entity = player;
					return intent;
					}
				case 'c':
					return {.type = Intent::Type::ShowPlayer};
				case 'h':
					return {.type = Intent::Type::Hide};
				case KEY_ESCAPE:
					registry.ctx().get<GameState>().running = false;
					return {.type = Intent::Type::DoNothing};
				case ' ':
					return {.type = Intent::Type::DoNothing};
				default:
					break;
			}
		}
		return {.type = Intent::Type::None};
	}

	void act_round(entt::registry& registry)
	{
		Cave& cave = ECS::get_active_cave(registry);
		const auto player = ECS::get_player(registry);
		std::vector<entt::entity> actors = ECS::get_entities(registry, Category("creatures"), cave);
		for (const auto actor : actors)
		{
			if (!registry.valid(actor)) continue;
			Intent intent = actor == player ?
				get_player_intent(registry) :
				AISystem::get_npc_intent(registry, actor);
			intent.actor = actor;
			resolve_intent(registry, intent);
			EventSystem::resolve_events(registry);
			if (cave.get_idx() != registry.get<Position>(player).cave_idx)
				return; // player left cave, end round
		}
	}

};
