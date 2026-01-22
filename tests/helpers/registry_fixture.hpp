#pragma once

class RegistryTest : public ::testing::Test
{
	protected:
		entt::registry registry;

		void SetUp() override
		{
			init_registry(registry);
		}
};
