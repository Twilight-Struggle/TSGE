#include "tsge/game_state/country.hpp"

#include <gtest/gtest.h>

#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/world_map_constants.hpp"

class CountryTest : public ::testing::Test {
 protected:
  CountryTest()
      : japan(CountryEnum::JAPAN, tsge::COUNTRY_STATIC_DATA[static_cast<size_t>(
                                      CountryEnum::JAPAN)]) {}

  Country japan;
};

TEST_F(CountryTest, AddInfluenceTest) {
  EXPECT_TRUE(japan.addInfluence(Side::USSR, 3));
  EXPECT_EQ(japan.getInfluence(Side::USSR), 3);
  EXPECT_EQ(japan.getInfluence(Side::USA), 0);

  EXPECT_TRUE(japan.addInfluence(Side::USA, 2));
  EXPECT_EQ(japan.getInfluence(Side::USA), 2);
  EXPECT_EQ(japan.getInfluence(Side::USSR), 3);

  EXPECT_FALSE(japan.addInfluence(Side::USSR, -1));
  EXPECT_EQ(japan.getInfluence(Side::USSR), 3);
}

TEST_F(CountryTest, RemoveInfluenceTest) {
  japan.addInfluence(Side::USSR, 5);
  japan.addInfluence(Side::USA, 3);

  EXPECT_TRUE(japan.removeInfluence(Side::USSR, 2));
  EXPECT_EQ(japan.getInfluence(Side::USSR), 3);

  EXPECT_TRUE(japan.removeInfluence(Side::USA, 5));
  EXPECT_EQ(japan.getInfluence(Side::USA), 0);

  EXPECT_FALSE(japan.removeInfluence(Side::USSR, -1));
  EXPECT_EQ(japan.getInfluence(Side::USSR), 3);
}

TEST_F(CountryTest, ClearInfluenceTest) {
  japan.addInfluence(Side::USSR, 3);
  japan.addInfluence(Side::USA, 2);

  EXPECT_TRUE(japan.clearInfluence(Side::USSR));
  EXPECT_EQ(japan.getInfluence(Side::USSR), 0);
  EXPECT_EQ(japan.getInfluence(Side::USA), 2);

  EXPECT_TRUE(japan.clearInfluence(Side::USA));
  EXPECT_EQ(japan.getInfluence(Side::USA), 0);
}

TEST_F(CountryTest, BasicPropertiesTest) {
  EXPECT_EQ(japan.getId(), CountryEnum::JAPAN);
  EXPECT_EQ(japan.getStability(), 4);
  EXPECT_TRUE(japan.isBattleground());
}

TEST_F(CountryTest, GetRegionsTest) {
  auto regions = japan.getRegions();
  EXPECT_FALSE(regions.empty());
  EXPECT_TRUE(japan.hasRegion(Region::ASIA));
  EXPECT_FALSE(japan.hasRegion(Region::EUROPE));
  EXPECT_FALSE(japan.hasRegion(Region::AFRICA));
}

TEST_F(CountryTest, GetAdjacentCountriesTest) {
  auto adjacent_countries = japan.getAdjacentCountries();
  EXPECT_FALSE(adjacent_countries.empty());

  bool has_usa = false;
  bool has_south_korea = false;
  for (const auto& country : adjacent_countries) {
    if (country == CountryEnum::USA) {
      has_usa = true;
    }
    if (country == CountryEnum::SOUTH_KOREA) {
      has_south_korea = true;
    }
  }
  EXPECT_TRUE(has_usa);
  EXPECT_TRUE(has_south_korea);
}

TEST_F(CountryTest, GetControlSideTest) {
  japan.clearInfluence(Side::USSR);
  japan.clearInfluence(Side::USA);
  EXPECT_EQ(japan.getControlSide(), Side::NEUTRAL);

  japan.addInfluence(Side::USSR, 4);
  EXPECT_EQ(japan.getControlSide(), Side::USSR);

  japan.addInfluence(Side::USA, 3);
  EXPECT_EQ(japan.getControlSide(), Side::NEUTRAL);

  japan.clearInfluence(Side::USSR);
  japan.clearInfluence(Side::USA);
  japan.addInfluence(Side::USA, 4);
  EXPECT_EQ(japan.getControlSide(), Side::USA);

  japan.clearInfluence(Side::USSR);
  japan.clearInfluence(Side::USA);
  japan.addInfluence(Side::USSR, 3);
  EXPECT_EQ(japan.getControlSide(), Side::NEUTRAL);
}