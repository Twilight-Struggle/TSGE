#include "trackers.hpp"

#include <gtest/gtest.h>

class SpaceTrackTest : public ::testing::Test {
 protected:
  SpaceTrack spaceTrack;
};

TEST_F(SpaceTrackTest, SpaceTrackTest) {
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 0));
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 1));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(Side::USSR, 1));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(Side::USSR, 1));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR));
}