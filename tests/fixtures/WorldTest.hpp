// tests/fixtures/WorldTest.hpp
#pragma once

#include <gtest/gtest.h>

#include "game/world/Grid.hpp"
#include "game/components/Component.hpp"
#include "external/entt/entt.hpp"

class WorldTest : public ::testing::Test
{
	protected:
		Game::World::Grid grid;
		entt::registry registry;
		entt::entity actor = entt::null;

		const Game::World::ChunkPosition chunk_position{};
		const Game::World::GlobalPosition center =
			Game::World::to_global(
					chunk_position,
					Game::World::LocalPosition{
					Game::World::Chunk::height / 2,
					Game::World::Chunk::width / 2});

		void SetUp() override
		{
			using namespace Game;

			grid.add(
					chunk_position,
					World::Chunk{Enum::Material::Stone, Enum::Form::Floor});

			actor = registry.create();
			registry.emplace<Component::Value::Position>(actor, center);
		}

		void reset_grid()
		{
			auto* chunk = grid.find_chunk(chunk_position);

			ASSERT_NE(chunk, nullptr)
				<< "Missing chunk at " << chunk_position;

			*chunk = Game::World::Chunk{
				Game::Enum::Material::Stone,
					Game::Enum::Form::Floor};
		}
};
