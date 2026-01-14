#pragma once

struct Damage
{
	enum class Type
	{
		None,
		Physical,
	};

	Type type;
	size_t amount;
};
