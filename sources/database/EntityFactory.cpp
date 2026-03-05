#include <algorithm>
#include <codecvt>
#include <filesystem>                                     // for path
#include <functional>
#include <limits>
#include <locale>
#include <map>
#include <nlohmann/detail/iterators/iter_impl.hpp>
#include <nlohmann/detail/iterators/iteration_proxy.hpp>
#include <nlohmann/json.hpp>                              // for basic_json
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <stddef.h>
#include <string>                                         // for string, ope...
#include <unordered_map>
#include <utility>
#include <vector>


#include "components/Components.hpp"                                 // for Resources
#include "core/paths.hpp"
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
	{ "glow", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Emits light around it
			if (!data.contains("intensity") || !data["intensity"].is_number() ||
				!data.contains("radius") || !data["radius"].is_number())
				Error::fatal("Glow requires intensity and radius numbers: " + data.dump(4));
			reg.template emplace<Glow>(e, data["intensity"].get<double>(), data["radius"].get<double>());
		}
	},
	{ "equipment_slot", [](auto& reg, auto e, const nlohmann::json& data)
		{	// The equipment slot where this is equipped
			if (!data.is_object())
				Error::fatal("Equipment slot should be object");

			using Slot = EquipmentSlot::Slot;

			EquipmentSlot es{};
			std::vector<Slot> slots;

			if (data["main_hand"].get<bool>())
				slots.push_back(Slot::MainHand);
			if (data["off_hand"].get<bool>())
				slots.push_back(Slot::OffHand);
			if (data["ammo"].get<bool>())
				slots.push_back(Slot::Ammo);

			const auto su = data["slot_usage"].get<std::string>();
			if (su == "use_one")
				es.use_one.emplace(slots);
			else if (su == "use_all")
				es.use_all.emplace(slots);
			else
				Error::fatal("Invalid slot_usage: " + su);

			reg.template emplace<EquipmentSlot>(e, es);
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
			AI ai;
			if (data["aggressive"].get<bool>())
				ai.aggressive = true;
			if (data["idle_wander"].get<bool>())
				ai.idle_wander = true;
			//if (data["predator"].get<bool>())
			//	ai.predator = true;
			reg.template emplace<AI>(e, ai);
		}
	},
	{ "transition", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (data.is_boolean() && data == true)
				reg.template emplace<Transition>(e, entt::null);
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
			assert(data.is_boolean() && "destroy_when_stacked should be boolean");
			reg.template emplace<DestroyWhenStacked>(e, data.get<bool>());
		}
	},
	{ "damage", [](auto& reg, auto e, const nlohmann::json& data)
		{
			Damage::Spec spec = Parser::parse_damage_spec(data);
			reg.template emplace<Damage::Spec>(e, spec);
		}
	},
	{ "attack_range", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const auto range = Parser::parse_range<double>(data);
			reg.template emplace<AttackRange>(e, range);
		}
	},
	{ "tags", [](auto& reg, auto e, const nlohmann::json& data)
		{
			assert(data.is_array());
			static const std::vector<std::string> ignored_tags = {
				"core", "spawns_naturally", "has_glow"
			};
			for (const auto& tag : data)
			{
				assert(tag.is_string());
				const std::string& str = tag.get<std::string>();
				if (std::find(ignored_tags.begin(), ignored_tags.end(), str) != ignored_tags.end())
					continue;
				if (str.size() < 3) continue;
				else if (str == "creature") reg.template emplace<Creature>(e);
				else if (str == "player") reg.template emplace<Player>(e);
				else if (str == "npc") reg.template emplace<NPC>(e);
				else if (str == "item") reg.template emplace<Item>(e);
				else if (str == "equipment") reg.template emplace<Equipment>(e);
				else if (str == "weapon") reg.template emplace<Weapon>(e);
				else if (str == "mechanical_weapon") reg.template emplace<MechanicalWeapon>(e);
				else if (str == "versatile_weapon") reg.template emplace<VersatileWeapon>(e);
				else if (str == "finesse_weapon") reg.template emplace<FinesseWeapon>(e);
				else if (str == "throwing_weapon") reg.template emplace<ThrowingWeapon>(e);
				else if (str == "ranged_weapon") reg.template emplace<RangedWeapon>(e);
				else if (str == "melee_weapon") reg.template emplace<MeleeWeapon>(e);
				else if (str == "improvised_weapon") reg.template emplace<ImprovisedWeapon>(e);
				else if (str == "gatherable") reg.template emplace<Gatherable>(e);
				else if (str == "mushroom") reg.template emplace<Mushroom>(e);
				else if (str == "plant") reg.template emplace <Plant>(e);
				else if (str == "tool") reg.template emplace<Tool>(e);
				else if (str == "ammo") reg.template emplace<Ammo>(e);
				else if (str == "door") reg.template emplace<Door>(e);
				else
					Error::fatal("Unhandled tag: " + str);
			}
		}
	},
	{ "environment_sensitive", [](auto& reg, auto e, const nlohmann::json& data)
		{
			(void) reg; (void) e; (void) data;
			// This data is already been processed and no longer needed
			// Affects spawning
			// Could be useful after spawning
		}
	},
	{ "gather_effect", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_string())
				Error::fatal("Gather effect is not string");
			const auto str = data.get<std::string>();
			GatherEffect effect{};
			if (str == "dim")
				effect = GatherEffect::Dim;
			else if (str == "destroy")
				effect = GatherEffect::Destroy;
			else
				Error::fatal("Unhandled gather effect: " + str);
			reg.template emplace<GatherEffect>(e, effect);
		}
	},
	{ "requires_ammo", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const AmmoType type = Parser::parse_ammo_type(data);
			reg.template emplace<RequiresAmmo>(e, type); // alias of AmmoType
		}
	},
	{ "ammo_type", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const AmmoType type = Parser::parse_ammo_type(data);
			reg.template emplace<AmmoType>(e, type);
		}
	},
	{ "requires_tool", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const ToolType type = Parser::parse_tool_type(data);
			reg.template emplace<RequiresTool>(e, type); // alias of ToolType
		}
	},
	{ "tool_type", [](auto& reg, auto e, const nlohmann::json& data)
		{
			const ToolType type = Parser::parse_tool_type(data);
			reg.template emplace<ToolType>(e, type);
		}
	},
	{ "loot_table_ref", [](auto& reg, auto e, const nlohmann::json& data)
		{
			if (!data.is_string())
				Error::fatal("Loot table ref not string");
			reg.template emplace<LootTableRef>(e, data.get<std::string>());
		}
	},
	{ "stackable", [](auto& reg, auto e, const nlohmann::json& data)
		{
			assert(data.is_boolean() && "stackable should be bool");
			reg.template emplace<Stackable>(e, data.get<bool>());
		}
	},
	{ "closed", [](auto& reg, auto e, const nlohmann::json& data)
		{
			assert(data.is_boolean() && "closed should be bool");
			reg.template emplace<Closed>(e, data.get<bool>());
		}
	},
	{ "mass", [](auto& reg, auto e, const nlohmann::json& data)
		{	// Verifying here is actually redundant, since Editor will notice these
			assert(data.is_number() && data.get<double>() >= 0 && "mass should be positive number");
			reg.template emplace<Mass>(e, data.get<double>());
		}
	}
};

/* Some components are ignored. They are probably part of spawning logic, and are not used anymore after creation.
 * */
bool ignored_component(const std::string& component)
{
	static const std::vector<std::string> ignored_components = {
		/* unused components */	"spawn_chance", "spawn_position"
	};
	auto it = std::find(ignored_components.begin(), ignored_components.end(), component);
	return it != ignored_components.end();
}

/* Make life easier and check all field parsers at once.
 * They have to handle every tag and component, unless ignored.
 * */
void EntityFactory::verify_field_parsers() const
{
	const auto component_definitions = Parser::read_json_file(component_definitions_file);

	std::string errors = "";
	for (const auto& [component, data] : component_definitions.items())
	{
		if (!field_parsers.contains(component) && !ignored_component(component))
			errors += "[unhandled component: " + component + "]";
	}
	if (!errors.empty())
		Error::fatal("Cannot verify field parsers: " + errors);
}

void EntityFactory::init()
{
	const std::filesystem::path file = "data/entities.json";
	nlohmann::json definitions = Parser::read_json_file(file);
	add_entities(definitions);
	Log::log("Entities parsed");
	verify_field_parsers();
}

void EntityFactory::add_entities(nlohmann::json& entities)
{
	for (const auto& entity : entities)
	{
		LUT[entity["name"].get<std::string>()] = entity;
	}
}

entt::entity EntityFactory::create_entity(entt::registry& registry, const std::string& name, const std::optional<Position>& position) const
{
	if (LUT.find(name) == LUT.end())
		Error::fatal("Entity not found: " + name);

	auto entity = registry.create();
	const auto& data = LUT.at(name);
	for (const auto& [field_name, field_data] : data.items())
	{
		if (ignored_component(field_name)) continue;
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

std::vector<entt::entity> EntityFactory::create_entities(entt::registry& registry, const nlohmann::json& filters) const
{
	const auto entity_ids = filter_entity_ids(filters);
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

std::vector<std::string> EntityFactory::filter_entity_ids(const nlohmann::json& filters) const
{
	Log::log("Filter: " + filters.dump(4));
	std::vector<std::string> ids;
	for (const auto& [id, data] : LUT)
	{
		bool include = true;
		for (const auto& [filter_type, filter_data] : filters.items())
		{
			if (filter_type == "contains_all" && !JsonUtils::contains_all(data, filter_data))
				include = false;
			else if (filter_type == "contains_any" && !JsonUtils::contains_any(data, filter_data))
				include = false;
			else if (filter_type == "contains_none" && !JsonUtils::contains_none(data, filter_data))
				include = false;

			if (include == false) break;
		}
		if (include)
			ids.push_back(id);
		Log::log("Include " + id + ": " + (include ? "true" : "false"));
	}
	return ids;
}

