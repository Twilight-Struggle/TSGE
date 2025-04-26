#include <gtest/gtest.h>

#include "cards.hpp"
#include "game.hpp"

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