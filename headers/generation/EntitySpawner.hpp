#pragma once

namespace entt
{
	using entity = std::uint32_t;
};

namespace EntitySpawner
{
	void spawn_entities(entt::registry& registry, const size_t cave_idx);
}
