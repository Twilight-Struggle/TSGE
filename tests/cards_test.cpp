#include "cards.hpp"

#include <gtest/gtest.h>

#include "game.hpp"

class DuckAndCoverTest : public ::testing::Test {
 protected:
  Game game;
  DuckAndCover sut;
};

TEST_F(DuckAndCoverTest, DuckAndCoverTest) {
  EXPECT_TRUE(sut.event(game, Side::USA));
  EXPECT_EQ(game.getVp(), -1);
  EXPECT_EQ(game.getDefconTrack().getDefcon(), 4);
}

class FidelTest : public ::testing::Test {
 protected:
  Game game;
  Fidel sut;
};

TEST_F(FidelTest, FidelTest) {
  EXPECT_TRUE(game.getWorldMap()
                  .getCountry(CountryEnum::CUBA)
                  .addInfluence(Side::USA, 1));
  EXPECT_TRUE(sut.event(game, Side::USSR));
  EXPECT_EQ(
      game.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USA),
      0);
  EXPECT_EQ(
      game.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USSR),
      3);
}

class NuclearTestBanTest : public ::testing::Test {
 protected:
  Game game;
  NuclearTestBan sut;
};

TEST_F(NuclearTestBanTest, NuclearTestBanTest) {
  EXPECT_TRUE(sut.event(game, Side::USSR));
  EXPECT_EQ(game.getVp(), 3);
  EXPECT_EQ(game.getDefconTrack().getDefcon(), 5);
  EXPECT_TRUE(game.getDefconTrack().setDefcon(2));
  EXPECT_TRUE(sut.event(game, Side::USA));
  EXPECT_EQ(game.getVp(), 3);
  EXPECT_EQ(game.getDefconTrack().getDefcon(), 4);
}