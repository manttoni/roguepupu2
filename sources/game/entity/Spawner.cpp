#include "game/entity/Spawner.hpp"
#include "game/entity/Entity.hpp"
#include "external/entt/entt.hpp"

namespace Game::Entity
{
	Spawner::Spawner(entt::registry& registry,
			const EntityDatabase& database,
			const World::Grid& grid) :
		registry(registry),
		database(database),
		grid(grid)
	{}

	entt::entity Spawner::spawn(
			const Definition::ID& id,
			const World::GlobalPosition& position)
	{
		if (!can_spawn(id, position))
			return entt::null;

		const auto entity = create(registry, database, id);
		registry.emplace<Component::Value::Position>(entity, position);
		return entity;
	}

	entt::entity Spawner::spawn(const Entity::SpawnRequest& request)
	{
		return spawn(request.id, request.position);
	}

	bool Spawner::can_spawn(
			const Definition::ID& id,
			const World::GlobalPosition& position
			) const
	{
		const auto* const cell = grid.find_cell(position);
		if (cell == nullptr)
			return false;

		const auto& data = database.definitions.at(id);
		const auto& tags = data.at("Tags");

		if (std::find(tags.begin(), tags.end(), "Plant") != tags.end())
		{
			// entity is a plant, it needs soil to grow
			if (cell->material != Enum::Material::Soil)
				return false;
		}

		if (cell->form != Enum::Form::Floor)
			return false; // TODO: Tags can change this restriction

		const auto blocking_entities = find_all(
				registry,
				Component::Value::Position{position},
				Component::Value::CollisionMovement{true}
				);

		const auto all_entities = find_all(
				registry,
				Component::Value::Position{position}
				);

		if (!blocking_entities.empty())
			return false;

		if (!all_entities.empty() && data.value("CollisionMovement", false))
			return false;

		return true;
	}

	bool Spawner::can_spawn(const Entity::SpawnRequest& request)
	{
		return can_spawn(request.id, request.position);
	}
}
