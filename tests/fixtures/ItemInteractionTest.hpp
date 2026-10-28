#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "databases/EntityDatabase.hpp"
#include "game/Simulation.hpp"
#include "game/component/Component.hpp"
#include "game/entity/Entity.hpp"

// Exercise complete event trees against the registry owned by Simulation.
// CTest runs from the repository root so the normal data files are available.
class ItemInteractionTest : public ::testing::Test
{
	protected:
		using Position = Game::World::GlobalPosition;
		using Inventory = Game::Component::List::Inventory;
		std::unique_ptr<Game::Simulation> simulation;
		entt::entity actor = entt::null;
		const Position center{
			Game::World::Chunk::height / 2,
			Game::World::Chunk::width / 2};

		void SetUp() override
		{
			simulation = std::make_unique<Game::Simulation>(
					"item-interactions-test", EntityDatabase{}, "player_default");
			actor = simulation->get_player();

			auto* chunk = simulation->get_grid().find_chunk(
					Game::World::to_chunk(center));
			ASSERT_NE(chunk, nullptr);
			*chunk = Game::World::Chunk{
				Game::Enum::Material::Stone, Game::Enum::Form::Floor};

			registry().replace<Game::Component::Value::Position>(actor, center);
			registry().get<Inventory>(actor).values.clear();
		}

		entt::registry& registry()
		{
			return simulation->get_registry();
		}

		const entt::registry& registry() const
		{
			return simulation->get_registry();
		}

		const std::vector<entt::entity>& inventory() const
		{
			return registry().get<Inventory>(actor).values;
		}

		entt::entity make_carried_item()
		{
			const auto item = Game::Entity::create(
					registry(), simulation->get_entity_database(), "stone");
			registry().get<Inventory>(actor).values.push_back(item);
			return item;
		}

		entt::entity make_ground_item(Position position)
		{
			const auto item = Game::Entity::create(
					registry(), simulation->get_entity_database(), "stone");
			registry().emplace<Game::Component::Value::Position>(item, position);
			return item;
		}

		void expect_carried(entt::entity item) const
		{
			ASSERT_TRUE(registry().valid(item));
			EXPECT_EQ(std::count(inventory().begin(), inventory().end(), item), 1);
			EXPECT_FALSE(registry().all_of<Game::Component::Value::Position>(item));
		}

		void expect_on_ground(entt::entity item, Position expected) const
		{
			ASSERT_TRUE(registry().valid(item));
			const auto* position =
				registry().try_get<Game::Component::Value::Position>(item);
			ASSERT_NE(position, nullptr);
			EXPECT_EQ(position->value, expected);
			EXPECT_EQ(std::count(inventory().begin(), inventory().end(), item), 0);
		}

		void expect_rejected(const Game::Event::Node& result) const
		{
			EXPECT_EQ(result.outcome, Game::Event::Outcome::Rejected);
			EXPECT_TRUE(result.consequences.empty());
		}
};
