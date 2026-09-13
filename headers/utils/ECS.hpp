#pragma once

#include "database/EntityDatabase.hpp"


#include <optional>
#include "rendering/Renderer.hpp"
#include "components/Component.hpp"
#include "domain/Cave.hpp"
#include "ncurses/Color.hpp"
#include "domain/Event.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/DevSettings.hpp"
#include "infrastructure/EventQueue.hpp"
#include "infrastructure/GameLogger.hpp"
#include "infrastructure/GameSettings.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/Vision.hpp"
#include "systems/Transition.hpp"
#include "systems/State.hpp"
#include "utils/Parser.hpp"
#include "utils/Random.hpp"
#include "utils/Utils.hpp"

namespace ECS
{
	/* These two functions return all entities either in a whole cave, or just in one cell in a cave
	 * */
	template <typename T = Domain::Position> std::vector<entt::entity> get_entities(const entt::registry& registry, const size_t cave_idx)
	{
		std::vector<entt::entity> entities;
		for (const auto e : registry.view<T>())
			if (registry.all_of<Domain::Position>(e) && registry.get<Domain::Position>(e).cave_idx == cave_idx)
				entities.push_back(e);
		return entities;
	}
	template <typename T = Domain::Position> std::vector<entt::entity> get_entities(const entt::registry& registry, const Domain::Position& pos)
	{
		std::vector<entt::entity> entities;
		for (const auto e : registry.view<T>())
			if (registry.all_of<Domain::Position>(e) && registry.get<Domain::Position>(e) == pos)
				entities.push_back(e);
		return entities;
	}
	template<typename T>
		T get_setting_value(const entt::registry& registry, GameSettings::Type type)
		{
			return std::get<T>(
					registry.ctx()
					.get<GameSettings>()
					.settings.at(type)
					.value
					);
		}

	inline Domain::Position get_position(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Domain::Position>(entity))
			return registry.get<Domain::Position>(entity);

		// Figure out a way to locate entity if it is carried.
		//if (registry.all_of<CarriedBy>(entity))
		//	return get_position(registry, registry.get<CarriedBy>(entity).value);

		return Domain::Position::invalid();
	}

	inline entt::entity get_player(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().player;
	}

	inline Domain::Position get_player_position(const entt::registry& registry)
	{
		return registry.get<Domain::Position>(get_player(registry));
	}

	inline Ncurses::Color get_fgcolor(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Ncurses::Color>(entity))
			return Ncurses::Color{3,3,3};
		return registry.get<Ncurses::Color>(entity);
	}

	inline std::string get_name(const entt::registry& registry, const entt::entity entity)
	{
		return registry.get<Component::Value::Name>(entity).value;
	}

	inline Ncurses::Attribute get_ncurses_attr(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Ncurses::Attribute>(entity))
			return registry.get<Ncurses::Attribute>(entity);
		return Ncurses::Attribute(A_NORMAL);
	}

	inline std::string get_colored_name(const entt::registry& registry, const entt::entity entity)
	{
		const Ncurses::Color& fgcolor = get_fgcolor(registry, entity);
		const Ncurses::Attribute& attr = get_ncurses_attr(registry, entity);
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

	inline void queue_event(entt::registry& registry, Domain::Event::Any event)
	{
		registry.ctx().get<EventQueue>().queue.push_back(std::move(event));
	}

	inline Domain::World& get_world(entt::registry& registry)
	{
		return registry.ctx().get<Domain::World>();
	}

	inline Domain::Cave& get_cave(entt::registry& registry, const Domain::Position& position)
	{
		return get_world(registry).get_cave(position.cave_idx);
	}

	inline Domain::Cell& get_cell(entt::registry& registry, const Domain::Position& position)
	{
		return get_cave(registry, position).get_cell(position);
	}

	inline const Domain::World& get_world(const entt::registry& registry)
	{
		return registry.ctx().get<Domain::World>();
	}

	inline const Domain::Cave& get_cave(const entt::registry& registry, const Domain::Position& position)
	{
		return get_world(registry).get_cave(position.cave_idx);
	}

	inline const Domain::Cell& get_cell(const entt::registry& registry, const Domain::Position& position)
	{
		return get_cave(registry, position).get_cell(position);
	}

	inline const Domain::Cave& get_cave(const entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<Domain::World>().get_cave(cave_idx);
	}

	inline Domain::Cave& get_cave(entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<Domain::World>().get_cave(cave_idx);
	}

	inline Domain::Cave& get_active_cave(entt::registry& registry)
	{
		const auto player = get_player(registry);
		assert(player != entt::null);
		assert(registry.all_of<Domain::Position>(player));
		const auto& pos = registry.get<Domain::Position>(player);
		return get_cave(registry, pos);
	}

	inline const Domain::Cave& get_active_cave(const entt::registry& registry)
	{
		const auto player = get_player(registry);
		assert(player != entt::null);
		assert(registry.all_of<Domain::Position>(player));
		const auto& pos = registry.get<Domain::Position>(player);
		return get_cave(registry, pos);
	}

	inline size_t get_turn_number(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().turn_number;
	}

	inline void destroy_entity(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<Component::Tag::Destroyed>(entity);

		queue_event(
				registry,
				Domain::Event::Destroy{
				.entity = entity
				});
	}

	inline double distance(const entt::registry& registry, const Domain::Position& a, const Domain::Position& b)
	{
		assert(a.cave_idx == b.cave_idx);
		return get_cave(registry, a).distance(a, b);
	}

	inline double distance(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		assert(registry.all_of<Domain::Position>(a) && registry.all_of<Domain::Position>(b));
		return distance(registry, registry.get<Domain::Position>(a), registry.get<Domain::Position>(b));
	}

	inline double distance(const entt::registry& registry, const entt::entity a, const Domain::Position& b)
	{
		return distance(registry, registry.get<Domain::Position>(a), b);
	}

	inline double distance(const entt::registry& registry, const Domain::Position& a, const entt::entity b)
	{
		return distance(registry, a, registry.get<Domain::Position>(b));
	}

	inline char get_glyph(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Component::Value::Glyph>(entity))
			return registry.get<Component::Value::Glyph>(entity).value;
		return registry.get<Component::Value::Name>(entity).value[0];
	}

	inline void init_registry(entt::registry& registry)
	{
		registry.ctx().emplace<GameState>();
		registry.ctx().emplace<GameLogger>();
		registry.ctx().emplace<Domain::World>();
		registry.ctx().emplace<EventQueue>();
		registry.ctx().emplace<GameSettings>();
		registry.ctx().emplace<EntityDatabase>();
		registry.ctx().emplace<Renderer>();
	}

	inline bool player_can_see_position(const entt::registry& registry, const Domain::Position& position)
	{
		const auto player = get_player(registry);
		return System::Vision::has_vision(registry, player, position);
	}

	inline bool player_can_see_entity(const entt::registry& registry, const entt::entity e)
	{
		if (get_player(registry) == entt::null || e == entt::null)
			return false;
		return System::Vision::has_vision(registry, get_player(registry), e);
	}

	inline bool game_running(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().game_running;
	}

	inline GameLogger::Stream game_log(entt::registry& registry)
	{
		return registry.ctx()
			.get<GameLogger>()
			.stream(registry);
	}

	inline bool weapon_has_property(const entt::registry& registry, const entt::entity weapon, const Domain::Enum::WeaponProperty property)
	{
		for (const auto p : registry.get<Component::List::WeaponProperties>(weapon))
			if (p == property)
				return true;
		return false;
	}

	inline bool blocks_vision(const entt::registry& registry, const Domain::Position& position)
	{
		const auto& cell = get_cell(registry, position);
		if (cell.get_type() == Domain::Cell::Type::Rock)
			return true;
		for (const auto e : get_entities<Component::Value::CollisionVision>(registry, position))
		{
			if (registry.get<Component::Value::CollisionVision>(e).value == true)
				return true;
		}
		return false;
	}

	template<typename C>
		inline size_t count(const entt::registry& registry, const std::vector<entt::entity>& values)
		{
			size_t c = 0;
			for (const auto e : values)
			{
				if (registry.all_of<C>(e))
					c++;
			}
			return c;
		}

	template<typename C>
		inline size_t count(const entt::registry& registry, const Component::List::Base<entt::entity>& list)
		{
			return count<C>(registry, list.values);
		}

	template<typename C>
		inline std::vector<entt::entity> get(const entt::registry& registry, const std::vector<entt::entity>& values)
		{
			std::vector<entt::entity> entities;
			for (const auto e : values)
			{
				if (registry.all_of<C>(e))
					entities.push_back(e);
			}
			return entities;
		}

	/* Return a vector of entities with component C
	 * */
	template<typename C>
		inline std::vector<entt::entity> get(const entt::registry& registry, const Component::List::Base<entt::entity>& list)
		{
			return get<C>(registry, list.values);
		}

	inline EntityDatabase get_entity_db(const entt::registry& registry)
	{
		return registry.ctx().get<EntityDatabase>();
	}

	template <typename C>
		inline std::vector<std::string> get_entity_ids(const entt::registry& registry)
		{
			std::vector<std::string> ids;
			const auto& database = get_entity_db(registry);
			for (const auto& [id, data] : database.definitions)
			{
				if (data.contains(C::string))
				{
					ids.push_back(id);
					continue;
				}

				if (!data.contains("Tags"))
				{
					Log::error() << "Entity doesn't have Tags. Ignored.";
					continue;
				}

				const auto tags = data.at("Tags").get<std::vector<std::string>>();
				if (std::ranges::find(tags, C::string) != tags.end())
					ids.push_back(id);
			}
			return ids;
		}

	template<typename C> inline std::vector<std::string> get_entity_names(const entt::registry& registry)
	{
		std::vector<std::string> names;
		const auto ids = get_entity_ids<C>(registry);
		const auto& definitions = ECS::get_entity_db(registry).definitions;
		for (const auto id : ids)
		{
			const auto definition = definitions.at(id);
			const auto name = definition.at("Name").template get<std::string>();
			names.push_back(name);
		}
		return names;
	}

	inline std::vector<std::string> get_entity_ids(const entt::registry& registry)
	{
		std::vector<std::string> ids;
		const auto& database = get_entity_db(registry);
		for (const auto& [id, unused] : database.definitions)
			ids.push_back(id);
		return ids;
	}

	template<typename T>
		struct is_vector : std::false_type {};

	template<typename T, typename Allocator>
		struct is_vector<std::vector<T, Allocator>> : std::true_type {};

	template<typename T>
		inline constexpr bool is_vector_v = is_vector<T>::value;

	template<typename T>
		T parse_value(const Json& data)
		{
			if constexpr (Domain::Enum::GameEnum<T>)
			{
				return Domain::Enum::from_string<T>(data.get<std::string>());
			}
			else if constexpr (is_vector_v<T>)
			{
				T values;
				values.reserve(data.size());

				for (const auto& element : data)
					values.push_back(parse_value<typename T::value_type>(element));

				return values;
			}
			else if constexpr (std::same_as<T, char>)
			{
				return data.get<std::string>()[0];
			}
			else
			{
				return data.get<T>();
			}
		}

	inline bool emplace_component(
			entt::registry& registry,
			const entt::entity entity,
			const std::string& component_str,
			const Json& data)
	{
		if (component_str == "Ncurses::Color")
		{
			const auto channels = data.get<std::array<int, 3>>();
			registry.emplace<Ncurses::Color>(entity, channels[0], channels[1], channels[2]);
		}
		else if (component_str == "Domain::Dice")
			registry.emplace<Domain::Dice>(entity, data.get<std::string>());
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Component::Value::name>(entity, parse_value<type>(data)); \
		}
#include "components/Value.def"
#undef X
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Component::List::name>(entity, parse_value<std::vector<type>>(data)); \
		}
#include "components/List.def"
#undef X
#define X(name, type) \
		else if (component_str == #name) \
		{ \
			registry.emplace<Component::Resource::name>(entity, parse_value<type>(data)); \
		}
#include "components/Resource.def"
#undef X
		else
		{
			Log::warning() << "No component called \"" << component_str << "\" exists";
			return false; // There is no matching component
		}
		Log::debug() << "Emplaced component: " << component_str;
		return true;
	}

	inline bool emplace_tags(
			entt::registry& registry,
			const entt::entity entity,
			const std::vector<std::string>& tags
			)
	{
		for (const auto& tag : tags)
		{
			if (tag.empty())
				return false;
#define X(name) \
			else if (tag == #name) \
				registry.emplace<Component::Tag::name>(entity);
#include "components/Tag.def"
#undef X
			else
			{
				Log::warning() << "No tag called \"" << tag << "\" exists";
				return false;
			}
			Log::debug() << "Emplaced tag: " << tag;
		}
		return true;
	}

	inline entt::entity create_entity(entt::registry& registry, const Json& definition)
	{
		const auto entity = registry.create();
		for (const auto& [component_str, data] : definition.items())
		{
			if (component_str == "Tags")
				emplace_tags(registry, entity, data.get<std::vector<std::string>>());
			else
				emplace_component(registry, entity, component_str, data);
		}
		return entity;
	}

	inline entt::entity create_entity(entt::registry& registry, const std::string& id)
	{
		const auto& database = get_entity_db(registry);
		const auto& definition = database.definitions.at(id);
		return create_entity(registry, definition);
	}
};
