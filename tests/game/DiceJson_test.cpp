#include <stdexcept>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "game/Dice.hpp"
#include "game/component/Component.hpp"
#include "game/entity/Entity.hpp"

namespace
{
	using Json = nlohmann::json;

	TEST(DiceJsonTest, SerializesAnExpressionAsACanonicalString)
	{
		const Game::Dice dice{" d8 - 2 "};
		const Json data(dice);

		ASSERT_TRUE(data.is_string());
		EXPECT_EQ(data.get<std::string>(), "1d8-2");
		EXPECT_EQ(data.get<Game::Dice>(), dice);
	}

	TEST(DiceJsonTest, DefaultComponentValueRoundTripsAsZero)
	{
		const Game::Component::Value::Dice component{};
		const Json data(component.value);

		ASSERT_TRUE(data.is_string());
		EXPECT_EQ(data.get<std::string>(), "0");
		EXPECT_EQ(data.get<Game::Dice>(), component.value);
	}

	TEST(DiceJsonTest, RejectsNonStringValues)
	{
		EXPECT_THROW((Json(4).get<Game::Dice>()), Json::type_error);
		EXPECT_THROW((Json::array({"1d4"}).get<Game::Dice>()), Json::type_error);
	}

	TEST(DiceJsonTest, RejectsAnInvalidExpression)
	{
		EXPECT_THROW((Json("2d0").get<Game::Dice>()), std::invalid_argument);
	}

	TEST(DiceJsonTest, CreatesADamageRollComponentFromAnEntityDefinition)
	{
		entt::registry registry;
		const Game::Entity::Definition definition{
			.id = "test_damage_roll",
			.data = {
				{"Tags", Json::array({"DamageRoll"})},
				{"Dice", "2d6+1"},
				{"DamageType", "Bludgeoning"}
			}
		};

		ASSERT_TRUE(Game::Entity::valid_definition(definition.data));
		const auto entity = Game::Entity::create(registry, definition);
		ASSERT_TRUE((registry.all_of<
			Game::Component::Tag::DamageRoll,
			Game::Component::Value::Dice,
			Game::Component::Value::DamageType>(entity)));
		EXPECT_EQ(registry.get<Game::Component::Value::Dice>(entity).value,
			Game::Dice("2d6+1"));
		EXPECT_EQ(registry.get<Game::Component::Value::DamageType>(entity).value,
			Game::Enum::DamageType::Bludgeoning);
	}
}
