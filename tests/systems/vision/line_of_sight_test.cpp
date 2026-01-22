/* This tests ensures that
 * VisionSystem::has_line_of_sight(const entt::registry&, const Position&, const Position&)
 * works properly.
 *
 * It has to check that parameters are interchangeable, and some simple cases.
 * Due to more or less unrealistic circumstances sometimes there is no line of sight,
 * even if it looks like there should be, especially the walls.
 *
 * The function uses Bresenhams line drawing algorithm with some modifications
 * */

#include <gtest/gtest.h>


