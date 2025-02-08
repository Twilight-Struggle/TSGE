#include "world_map.hpp"

#include <gtest/gtest.h>

#include <cstddef>

#include "game_enums.hpp"

class WorldMapTest : public ::testing::Test {
 protected:
  WorldMap worldMap;
};

TEST_F(WorldMapTest, CountryIndexTest) {
  for (size_t i = static_cast<size_t>(CountryEnum::USSR);
       i <= static_cast<size_t>(CountryEnum::FINLAND); i++) {
    EXPECT_EQ(worldMap.getCountry(static_cast<CountryEnum>(i)).getId(),
              static_cast<CountryEnum>(i));
  }
}

// TEST_F(WorldMapTest, PlaceInfuenceTest) {
//   PlaceInfluence action({{CountryEnum::NORTH_KOREA, 1}});
//   action.execute(board, Side::USSR);
//   EXPECT_EQ(board.getCountry(CountryEnum::NORTH_KOREA).getInfluence(Side::USSR),
//             1);
// }
