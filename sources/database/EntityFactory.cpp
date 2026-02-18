#include <nlohmann/json.hpp>                              // for basic_json
#include <stddef.h>
#include <nlohmann/detail/iterators/iter_impl.hpp>
#include <nlohmann/detail/iterators/iteration_proxy.hpp>
#include <nlohmann/json_fwd.hpp>
#include <codecvt>
#include <locale>
#include <filesystem>                                     // for path
#include <string>                                         // for string, ope...
#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "components/Components.hpp"                                 // for Resources
#include "database/EntityFactory.hpp"                              // for EntityFactory
#include "domain/Ability.hpp"
#include "domain/Color.hpp"                                      // for Color
#include "domain/Effect.hpp"
#include "domain/Event.hpp"
#include "domain/Position.hpp"
#include "domain/Target.hpp"
#include "external/entt/entity/fwd.hpp"
#include "external/entt/entt.hpp"
#include "systems/items/LootSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "utils/JsonUtils.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"
#include "utils/Random.hpp"

class AbilityDatabase;

void EntityFactory::init()
{
	const std::filesystem::path file = "data/entities.json";
	nlohmann::json definitions = Parser::read_json_file(file);
	add_entities(definitions);
	Log::log("Entities parsed");
}
void EntityFactory::add_entities(nlohmann::json& entities)
{
	for (const auto& entity : entities)
	{
		LUT[entity["name"].get<std::string>()] = entity;
	}
}

using FieldParser = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;
std::unordered_map<std::string, FieldParser> field_parsers =
{
	{ "name", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Name>(e, data.get<std::string>());
		}
	},
	{ "solid", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Can't move through solid entities
			if (!data.is_boolean())
				Error::fatal("Solid should be boolean: " + data.dump(4));
			if (data.get<bool>() == true)
				reg.template emplace<Solid>(e);
		}
	},
	{ "opaque", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_number() || data.get<double>() < 0 || data.get<double>() > 1)
				Error::fatal("Opaqueness should be number [0,1]");
			reg.template emplace<Opaque>(e, data.get<double>());
		}
	},
	{ "inventory", [](auto& reg, auto e, const nlohmann::json& data)
		{
			std::vector<entt::entity> inventory;
			if (data.contains("loot_tables"))
			{
				for (const auto& table_id : data["loot_tables"])
				{
					const auto loot = LootSystem::get_loot(reg, table_id.get<std::string>());
					inventory.insert(inventory.end(), loot.begin(), loot.end());
				}
			}
			reg.template emplace<Inventory>(e, inventory);
		}
	},
	{ "vision", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Can see and has a vision range. This might get replaced by some perception stat, probably will
			if (!data.is_number() || data.get<double>() < 0)
				Error::fatal("Vision range should be positive number");
			reg.template emplace<Vision>(e, data.get<double>());
		}
	},
	{ "glow", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Emits light around it
			if (!data.contains("intensity") || !data["intensity"].is_number() ||
				!data.contains("radius") || !data["radius"].is_number())
				Error::fatal("Glow requires intensity and radius numbers: " + data.dump(4));
			reg.template emplace<Glow>(e, data["intensity"].get<double>(), data["radius"].get<double>());
		}
	},
	{ "player", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Is a playable character
			if (!data.is_boolean())
				Error::fatal("Player should be boolean: " + data.dump(4));
			if (data.get<bool>() == true)
				reg.template emplace<Player>(e);
		}
	},
	{ "weight", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_number() || data.get<double>() < 0)
				Error::fatal("Weight should be positive number: " + data.dump(4));
			reg.template emplace<Weight>(e, data.get<double>());
		}
	},
	{ "equipment", [](auto& reg, auto e, const nlohmann::json& data)
		{	// This entity is a piece of equipment that can be equipped in slots
			Equipment equipment;
			if (data.is_string())
			{
				const auto str = data.get<std::string>();
				if (str == "one_handed")
					equipment.use_one.emplace(std::vector<Equipment::Slot>{Equipment::Slot::MainHand, Equipment::Slot::OffHand});
				else if (str == "two_handed")
					equipment.use_all.emplace(std::vector<Equipment::Slot>{Equipment::Slot::MainHand, Equipment::Slot::OffHand});
				else
					equipment.use_all.emplace(std::vector<Equipment::Slot>{Equipment::slot_from_string(str)});
			}
			else if (data.is_object() && data.size() == 1)
			{
				auto it = data.items().begin();
				if (it == data.items().end())
					Error::fatal("Equipment slots parse error");
				const auto& key = it.key();
				const auto& value = it.value();
				std::vector<std::string> slot_strings = value.get<std::vector<std::string>>();
				std::vector<Equipment::Slot> slots;
				for (const auto& str : slot_strings)
					slots.push_back(Equipment::slot_from_string(str));
				if (key == "use_all")
					equipment.use_all.emplace(slots);
				else if (key == "use_one")
					equipment.use_one.emplace(slots);
				else
					Error::fatal("Invalid equipment component: " + data.dump(4));
			}
			else
				Error::fatal("Invalid equipment component: " + data.dump(4));
			reg.template emplace<Equipment>(e, equipment);
		}
	},
	{ "equipment_slots", [](auto& reg, auto e, const nlohmann::json& data)
		{	// can equip items
			(void) data;
			reg.template emplace<EquipmentSlots>(e);
		}
	},
	{ "glyph", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_string())
				Error::fatal("Glyph should be string: " + data.dump(4));
			const std::string glyph_str = data.get<std::string>();
			std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
			std::wstring wstr = conv.from_bytes(glyph_str);
			const wchar_t glyph = wstr[Random::rand<size_t>(0, wstr.size() - 1)];
			reg.template emplace<Glyph>(e, glyph);
		}
	},
	{ "color", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Color color = Parser::parse_color(data);
			reg.template emplace<Color>(e, color);
		}
	},
	{ "level", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_number() || data.get<int>() < 0)
				Error::fatal("Level should be positive integer: " + data.dump(4));

			const size_t xp = StateSystem::level_to_xp(data.get<size_t>());
			reg.template emplace<Experience>(e, xp);
		}
	},
	{ "abilities", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_array())
				Error::fatal("Abilities not an array: " + data.dump(4));
			std::map<std::string, Ability> abilities;
			for (const auto& ability : data)
			{
				const auto id = ability.get<std::string>();
				const auto& database = reg.ctx().template get<AbilityDatabase>();
				abilities[id] = database.get_ability(id);
			}
			reg.template emplace<Abilities>(e, abilities);
		}
	},
	{ "ai", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_object())
				Error::fatal("AI needs to be an object: " + data.dump(4));
			AI comp;
			if (data.contains("aggressive")) comp.aggressive = true;
			if (data.contains("idle_wander")) comp.idle_wander = true;

			reg.template emplace<AI>(e, comp);
		}
	},
	{ "spawn", [](auto& reg, auto e, const nlohmann::json& data)
		{	// unused at this point
			(void) reg; (void) e; (void) data;
		}
	},
	{ "gatherable", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Gatherable comp;
			if (data.contains("tool"))
				comp.tool_type = Tool::from_string(data["tool"].get<std::string>());
			if (data.contains("gather_effect"))
				comp.effect = Gatherable::from_string(data["gather_effect"].get<std::string>());
			if (data.contains("loot_tables"))
				comp.loot_table_ids = data["loot_tables"].get<std::vector<std::string>>();

			reg.template emplace<Gatherable>(e, comp);
		}
	},
	{ "tool", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<Tool>(e, Tool::from_string(data.get<std::string>()));
		}
	},
	{ "transition", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (data.is_boolean() && data == true)
				reg.template emplace<Transition>(e, entt::null);
		}
	},
	{ "size", [](auto& reg, auto e, const nlohmann::json& data)
		{
			double size = 0;
			if (data.is_string() && data.get<std::string>() == "-inf")
				size = -std::numeric_limits<double>::infinity();
			else if (data.is_string() && data.get<std::string>() == "inf")
				size = std::numeric_limits<double>::infinity();
			else if (data.is_number())
				size = data.get<double>();
			else Error::fatal("Unhandled size type: " + data.dump(4));
			reg.template emplace<Size>(e, size);
		}
	},
	{ "attributes", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_object())
				Error::fatal("Incorrect format: " + data.dump(4));
			if (data.contains("endurance"))
			{
				reg.template emplace<Endurance>(e, data["endurance"].get<int>());
				reg.template emplace<Stamina>(e, StateSystem::get_max_stamina(reg, e));
			}
			if (data.contains("willpower"))
			{
				reg.template emplace<Willpower>(e, data["willpower"].get<int>());
				reg.template emplace<Mana>(e, StateSystem::get_max_mana(reg, e));
			}
			if (data.contains("vitality"))
			{
				reg.template emplace<Vitality>(e, data["vitality"].get<int>());
				reg.template emplace<Health>(e, StateSystem::get_max_health(reg, e));
			}
			if (data.contains("perception"))
				reg.template emplace<Perception>(e, data["perception"].get<int>());
			if (data.contains("charisma"))
				reg.template emplace<Charisma>(e, data["charisma"].get<int>());
			if (data.contains("strength"))
				reg.template emplace<Strength>(e, data["strength"].get<int>());
			if (data.contains("dexterity"))
				reg.template emplace<Dexterity>(e, data["dexterity"].get<int>());
			if (data.contains("agility"))
				reg.template emplace<Agility>(e, data["agility"].get<int>());
		}
	},
	{ "health", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Entities can have health, even without vitality. They just cannot heal/be repaired
			reg.template emplace<Health>(e, data.get<int>());
		}
	},
	{ "liquid_container", [](auto& reg, auto e, const nlohmann::json& data)
		{
			reg.template emplace<LiquidContainer>(e, data["capacity"].get<double>());
		}
	},
	{ "alignment", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Alignment alignment;
			alignment.tolerance = data["tolerance"].get<double>();
			alignment.chaos_law = data["chaos_law"].get<double>();
			alignment.evil_good = data["evil_good"].get<double>();
			reg.template emplace<Alignment>(e, alignment);
		}
	},
	{ "destroy_when_stacked", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<DestroyWhenStacked>(e);
		}
	},
	{ "damage", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Damage::Spec spec = Parser::parse_damage_spec(data);
			reg.template emplace<Damage::Spec>(e, spec);
		}
	},
	{ "melee_weapon", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<MeleeWeapon>(e);
		}
	},
	{ "ranged_weapon", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<RangedWeapon>(e);
		}
	},
	{ "throwing_weapon", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<ThrowingWeapon>(e);
		}
	},
	{ "finesse_weapon", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<FinesseWeapon>(e);
		}
	},
	{ "versatile_weapon", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<VersatileWeapon>(e);
		}
	},
	{ "mechanical_weapon", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) data;
			reg.template emplace<MechanicalWeapon>(e);
		}
	},
	{ "range", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const auto range = Parser::parse_range<double>(data);
			reg.template emplace<AttackRange>(e, range);
		}
	},
	{ "tags", [](auto& reg, auto e, const nlohmann::json& data)
		{
			assert(data.is_array());
			for (const auto& tag : data)
			{
				assert(tag.is_string());
				const std::string& str = tag.get<std::string>();
				if (str == "creature")
					reg.template emplace<Creature>(e);
				else if (str == "weapon")
					reg.template emplace<Weapon>(e);
				else if (str == "player")
					reg.template emplace<Player>(e);
				else if (str == "npc")
					reg.template emplace<NPC>(e);
				else if (str == "core" || str == "spawns_naturally" || str == "has_glow")
					continue;
				else
					Error::fatal("Unhandled tag: " + str);
			}
		}
	},
	{ "spawn_chance", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) reg; (void) e; (void) data;
			// This data is already been processed and no longer needed
		}
	},
	{ "environment_sensitive", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) reg; (void) e; (void) data;
			// This data is already been processed and no longer needed
			// Affects spawning
			// Could be useful after spawning
		}
	}
};

entt::entity EntityFactory::create_entity(entt::registry& registry, const std::string& name, const std::optional<Position>& position) const
{
	if (LUT.find(name) == LUT.end())
		Error::fatal("Entity not found: " + name);

	auto entity = registry.create();
	const auto& data = LUT.at(name);
	for (const auto& [field_name, field_data] : data.items())
	{
		auto it = field_parsers.find(field_name);
		if (it == field_parsers.end())
			Error::fatal("Unknown field name: " + field_name);
		try {
			it->second(registry, entity, field_data);
		} catch (const nlohmann::json::parse_error& e) {
			Error::fatal(e.what());
		}
	}

	if (position.has_value())
	{
		registry.emplace<Position>(entity, *position);
		Event spawn_event;
		spawn_event.type = Event::Type::Spawn;
		spawn_event.target.entity = entity;
		spawn_event.target.position = *position;
		ECS::queue_event(registry, spawn_event);
	}

	return entity;
}

std::vector<entt::entity> EntityFactory::create_entities(entt::registry& registry, const nlohmann::json& include, const nlohmann::json& exclude) const
{
	const auto entity_ids = filter_entity_ids(include, exclude);
	return create_entities(registry, entity_ids);
}

std::vector<entt::entity> EntityFactory::create_entities(entt::registry& registry, const std::vector<std::string>& entity_ids) const
{
	std::vector<entt::entity> entities;
	entities.reserve(entity_ids.size());
	for (const auto& id : entity_ids)
	{
		const auto entity = create_entity(registry, id);
		entities.push_back(entity);
	}
	return entities;
}

std::vector<entt::entity> EntityFactory::create_entities(entt::registry& registry, const std::string& id, const size_t amount) const
{
	std::vector<entt::entity> entities;
	for (size_t i = 0; i < amount; ++i)
	{
		const auto e = create_entity(registry, id);
		entities.push_back(e);
	}
	return entities;
}

std::vector<std::string> EntityFactory::filter_entity_ids(const nlohmann::json& include, const nlohmann::json& exclude) const
{
	std::vector<std::string> ids;
	for (const auto& [name, data] : LUT)
	{
		if (JsonUtils::contains_all(data, include) && JsonUtils::contains_none(data, exclude))
			ids.push_back(name);
	}
	std::sort(ids.begin(), ids.end());
	return ids;
}
