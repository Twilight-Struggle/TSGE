#include "trackers.hpp"

#include <gtest/gtest.h>

#include "game.hpp"

class SpaceTrackTest : public ::testing::Test {
 protected:
  SpaceTrack spaceTrack;
  Game game;
};

TEST_F(SpaceTrackTest, SpaceTrackTest) {
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 0));
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 1));
  EXPECT_TRUE(spaceTrack.canSpaceChallenge(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpaceChallenge(Side::USSR));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(game, Side::USSR, 1));
  EXPECT_EQ(game.getVp(), 2);
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(game, Side::USSR, 1));
  EXPECT_EQ(game.getVp(), 2);
  EXPECT_TRUE(spaceTrack.canSpaceChallenge(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpaceChallenge(Side::USSR));
}

TEST_F(SpaceTrackTest, SpaceTrackCanSpaceTest) {
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 2));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(game, Side::USSR, 4));
  EXPECT_EQ(game.getVp(), 0);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 2));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 3));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(game, Side::USSR, 3));
  EXPECT_EQ(game.getVp(), 4);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 3));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 4));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(game, Side::USSR, 1));
  EXPECT_EQ(game.getVp(), 6);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 4));
}