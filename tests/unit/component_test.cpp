#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "core/paths.hpp"

TEST(ComponentTest, ComponentsAreDefined)
{
	const auto component_definitions = Parser::read_json_file(component_definitions_file);
	const auto tag_dependencies = Parser::read_json_file(tag_dependencies_file);
	for (const auto [tag, dependencies] : tag_dependencies.items())
	{
		if (!dependencies.contains("components"))
			continue;
		ASSERT_TRUE(dependencies["components"].is_array());
		for (const auto component : dependencies["components"])
			EXPECT_TRUE(component_definitions.contains(component.get<std::string>()));
	}
}

TEST(ComponentTest, TagsAreDefined)
{
	const auto tag_dependencies = Parser::read_json_file(tag_dependencies_file);
	for (const auto [tag, dependencies] : tag_dependencies.items())
	{
		if (!dependencies.contains("tags"))
			continue;
		ASSERT_TRUE(dependencies["tags"].is_array());
		for (const auto tag : dependencies["tags"])
			EXPECT_TRUE(tag_dependencies.contains(tag.get<std::string>()));
	}
}
