#include "trackers.hpp"

#include <gtest/gtest.h>

#include "game.hpp"

class TrackTest : public ::testing::Test {
 protected:
  Game game;
  SpaceTrack spaceTrack;
  DefconTrack defconTrack{game};
};

TEST_F(TrackTest, SpaceTrackTest) {
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

TEST_F(TrackTest, SpaceTrackCanSpaceTest) {
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

TEST_F(TrackTest, DefconTrackTest) {
  EXPECT_TRUE(defconTrack.setDefcon(4));
  EXPECT_FALSE(defconTrack.setDefcon(0));
  EXPECT_FALSE(defconTrack.setDefcon(6));
  EXPECT_TRUE(defconTrack.changeDefcon(-1));
  EXPECT_EQ(defconTrack.getDefcon(), 3);
}