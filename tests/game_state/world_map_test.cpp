#include "tsge/game_state/world_map.hpp"

#include <gtest/gtest.h>

#include <cstddef>

#include "tsge/enums/game_enums.hpp"

class WorldMapTest : public ::testing::Test {
 protected:
  WorldMap worldMap;
};

TEST_F(WorldMapTest, CountryIndexTest) {
  for (auto i = static_cast<size_t>(CountryEnum::USSR);
       i <= static_cast<size_t>(CountryEnum::FINLAND); i++) {
    EXPECT_EQ(worldMap.getCountry(static_cast<CountryEnum>(i)).getId(),
              static_cast<CountryEnum>(i));
  }
}

TEST_F(WorldMapTest, RegionIncludeExactlyTest) {
  auto central_america = worldMap.countriesInRegion(Region::CENTRAL_AMERICA);
  EXPECT_TRUE(
      central_america.contains(worldMap.getCountry(CountryEnum::MEXICO)));
  EXPECT_TRUE(central_america.contains(worldMap.getCountry(CountryEnum::CUBA)));
  EXPECT_TRUE(
      central_america.contains(worldMap.getCountry(CountryEnum::PANAMA)));
  auto south_america = worldMap.countriesInRegion(Region::SOUTH_AMERICA);
  EXPECT_TRUE(
      south_america.contains(worldMap.getCountry(CountryEnum::VENEZUELA)));
  EXPECT_TRUE(south_america.contains(worldMap.getCountry(CountryEnum::CHILE)));
  EXPECT_TRUE(
      south_america.contains(worldMap.getCountry(CountryEnum::ARGENTINA)));
  EXPECT_TRUE(south_america.contains(worldMap.getCountry(CountryEnum::BRAZIL)));
  auto africa = worldMap.countriesInRegion(Region::AFRICA);
  EXPECT_TRUE(africa.contains(worldMap.getCountry(CountryEnum::ALGERIA)));
  EXPECT_TRUE(africa.contains(worldMap.getCountry(CountryEnum::NIGERIA)));
  EXPECT_TRUE(africa.contains(worldMap.getCountry(CountryEnum::ZAIRE)));
  EXPECT_TRUE(africa.contains(worldMap.getCountry(CountryEnum::ANGOLA)));
  EXPECT_TRUE(africa.contains(worldMap.getCountry(CountryEnum::SOUTH_AFRICA)));
  auto middle_east = worldMap.countriesInRegion(Region::MIDDLE_EAST);
  EXPECT_TRUE(middle_east.contains(worldMap.getCountry(CountryEnum::LIBYA)));
  EXPECT_TRUE(middle_east.contains(worldMap.getCountry(CountryEnum::EGYPT)));
  EXPECT_TRUE(middle_east.contains(worldMap.getCountry(CountryEnum::ISRAEL)));
  EXPECT_TRUE(middle_east.contains(worldMap.getCountry(CountryEnum::IRAQ)));
  EXPECT_TRUE(middle_east.contains(worldMap.getCountry(CountryEnum::IRAN)));
  EXPECT_TRUE(
      middle_east.contains(worldMap.getCountry(CountryEnum::SAUDI_ARABIA)));
  auto asia = worldMap.countriesInRegion(Region::ASIA);
  EXPECT_TRUE(asia.contains(worldMap.getCountry(CountryEnum::PAKISTAN)));
  EXPECT_TRUE(asia.contains(worldMap.getCountry(CountryEnum::INDIA)));
  EXPECT_TRUE(asia.contains(worldMap.getCountry(CountryEnum::THAILAND)));
  EXPECT_TRUE(asia.contains(worldMap.getCountry(CountryEnum::JAPAN)));
  EXPECT_TRUE(asia.contains(worldMap.getCountry(CountryEnum::SOUTH_KOREA)));
  EXPECT_TRUE(asia.contains(worldMap.getCountry(CountryEnum::NORTH_KOREA)));
  auto europe = worldMap.countriesInRegion(Region::EUROPE);
  EXPECT_TRUE(europe.contains(worldMap.getCountry(CountryEnum::POLAND)));
  EXPECT_TRUE(europe.contains(worldMap.getCountry(CountryEnum::EAST_GERMANY)));
  EXPECT_TRUE(europe.contains(worldMap.getCountry(CountryEnum::WEST_GERMANY)));
  EXPECT_TRUE(europe.contains(worldMap.getCountry(CountryEnum::FRANCE)));
  EXPECT_TRUE(europe.contains(worldMap.getCountry(CountryEnum::ITALY)));
  auto east_europe = worldMap.countriesInRegion(Region::EAST_EUROPE);
  EXPECT_TRUE(east_europe.contains(worldMap.getCountry(CountryEnum::POLAND)));
  EXPECT_TRUE(
      east_europe.contains(worldMap.getCountry(CountryEnum::EAST_GERMANY)));
  EXPECT_TRUE(east_europe.contains(worldMap.getCountry(CountryEnum::AUSTRIA)));
  EXPECT_TRUE(east_europe.contains(worldMap.getCountry(CountryEnum::FINLAND)));
  auto west_europe = worldMap.countriesInRegion(Region::WEST_EUROPE);
  EXPECT_TRUE(
      west_europe.contains(worldMap.getCountry(CountryEnum::WEST_GERMANY)));
  EXPECT_TRUE(west_europe.contains(worldMap.getCountry(CountryEnum::FRANCE)));
  EXPECT_TRUE(west_europe.contains(worldMap.getCountry(CountryEnum::ITALY)));
  EXPECT_TRUE(west_europe.contains(worldMap.getCountry(CountryEnum::AUSTRIA)));
  EXPECT_TRUE(west_europe.contains(worldMap.getCountry(CountryEnum::FINLAND)));
  auto south_east_asia = worldMap.countriesInRegion(Region::SOUTH_EAST_ASIA);
  EXPECT_TRUE(
      south_east_asia.contains(worldMap.getCountry(CountryEnum::THAILAND)));
  EXPECT_TRUE(
      south_east_asia.contains(worldMap.getCountry(CountryEnum::VIETNAM)));
  EXPECT_TRUE(
      south_east_asia.contains(worldMap.getCountry(CountryEnum::PHILIPPINES)));
  EXPECT_TRUE(
      south_east_asia.contains(worldMap.getCountry(CountryEnum::INDONESIA)));
  EXPECT_TRUE(
      south_east_asia.contains(worldMap.getCountry(CountryEnum::MALAYSIA)));
  auto special = worldMap.countriesInRegion(Region::SPECIAL);
  EXPECT_TRUE(special.contains(worldMap.getCountry(CountryEnum::USSR)));
  EXPECT_TRUE(special.contains(worldMap.getCountry(CountryEnum::USA)));
}

TEST_F(WorldMapTest, PlaceableCountriesTest) {
  auto placeable_ussr = worldMap.placeableCountries(Side::USSR);
  EXPECT_FALSE(placeable_ussr.empty());

  EXPECT_FALSE(placeable_ussr.contains(CountryEnum::USSR));
  EXPECT_FALSE(placeable_ussr.contains(CountryEnum::USA));

  EXPECT_TRUE(placeable_ussr.contains(CountryEnum::EAST_GERMANY));
  EXPECT_TRUE(placeable_ussr.contains(CountryEnum::POLAND));

  auto placeable_usa = worldMap.placeableCountries(Side::USA);
  EXPECT_FALSE(placeable_usa.empty());

  EXPECT_FALSE(placeable_usa.contains(CountryEnum::USSR));
  EXPECT_FALSE(placeable_usa.contains(CountryEnum::USA));

  EXPECT_TRUE(placeable_usa.contains(CountryEnum::CANADA));
  EXPECT_TRUE(placeable_usa.contains(CountryEnum::JAPAN));

  worldMap.getCountry(CountryEnum::JAPAN).addInfluence(Side::USSR, 1);
  auto placeable_after_japan = worldMap.placeableCountries(Side::USSR);
  EXPECT_TRUE(placeable_after_japan.contains(CountryEnum::JAPAN));
  EXPECT_TRUE(placeable_after_japan.contains(CountryEnum::SOUTH_KOREA));
}

TEST_F(WorldMapTest, ConstGetCountryTest) {
  const WorldMap& const_world_map = worldMap;
  const Country& country = const_world_map.getCountry(CountryEnum::JAPAN);
  EXPECT_EQ(country.getId(), CountryEnum::JAPAN);
}

TEST_F(WorldMapTest, CountsTest) {
  EXPECT_EQ(worldMap.getCountriesCount(), 86);
  EXPECT_EQ(worldMap.getRegionsCount(), 10);
}

// TEST_F(WorldMapTest, PlaceInfuenceTest) {
//   PlaceInfluence action({{CountryEnum::NORTH_KOREA, 1}});
//   action.execute(board, Side::USSR);
//   EXPECT_EQ(board.getCountry(CountryEnum::NORTH_KOREA).getInfluence(Side::USSR),
//             1);
// }
