#pragma once

#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"

class RegistryTest : public ::testing::Test
{
	protected:
		entt::registry registry;

		void SetUp() override
		{
			ECS::init_registry(registry);
		}
};
