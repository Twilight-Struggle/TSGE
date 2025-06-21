#include "tsge/game_state/trackers.hpp"

#include <gtest/gtest.h>

#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"

class TrackTest : public ::testing::Test {
 protected:
  TrackTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }

  Board board;
  SpaceTrack spaceTrack;
  DefconTrack defconTrack;
  MilopsTrack milopsTrack;
  TurnTrack turnTrack;
};

TEST_F(TrackTest, SpaceTrackTest) {
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 0));
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 1));
  EXPECT_TRUE(spaceTrack.canSpaceChallenge(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpaceChallenge(Side::USSR));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(Side::USSR, 1));
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 1);
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(Side::USSR, 1));
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 2);
  EXPECT_TRUE(spaceTrack.canSpaceChallenge(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpaceChallenge(Side::USSR));
}

TEST_F(TrackTest, SpaceTrackCanSpaceTest) {
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 2));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(Side::USSR, 4));
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 4);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 2));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 3));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(Side::USSR, 3));
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 7);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 3));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 4));
  EXPECT_TRUE(spaceTrack.advanceSpaceTrack(Side::USSR, 1));
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 8);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 4));
}

TEST_F(TrackTest, DefconTrackTest) {
  EXPECT_TRUE(defconTrack.setDefcon(4));
  EXPECT_EQ(defconTrack.getDefcon(), 4);
  // setDefconは範囲チェックを行わない（合法手判断はLegalMovesGeneratorで行う）
  EXPECT_TRUE(defconTrack.setDefcon(0));
  EXPECT_EQ(defconTrack.getDefcon(), 0);
  EXPECT_TRUE(defconTrack.setDefcon(6));
  EXPECT_EQ(defconTrack.getDefcon(), 6);
  // changeDefconは範囲をクランプする
  defconTrack.setDefcon(4);
  EXPECT_TRUE(defconTrack.changeDefcon(-1));
  EXPECT_EQ(defconTrack.getDefcon(), 3);
}

TEST_F(TrackTest, MilopsTrackTest) {
  EXPECT_TRUE(milopsTrack.advanceMilopsTrack(Side::USSR, 2));
  EXPECT_EQ(milopsTrack.getMilops(Side::USSR), 2);
  EXPECT_TRUE(milopsTrack.advanceMilopsTrack(Side::USA, 3));
  EXPECT_EQ(milopsTrack.getMilops(Side::USA), 3);
  EXPECT_TRUE(milopsTrack.resetMilopsTrack());
  EXPECT_EQ(milopsTrack.getMilops(Side::USSR), 0);
  EXPECT_EQ(milopsTrack.getMilops(Side::USA), 0);
  EXPECT_TRUE(milopsTrack.advanceMilopsTrack(Side::USSR, 7));
  EXPECT_EQ(milopsTrack.getMilops(Side::USSR), 5);
}

TEST_F(TrackTest, TurnTrackTest) {
  EXPECT_EQ(turnTrack.getTurn(), 1);
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_EQ(turnTrack.getTurn(), 2);
  EXPECT_EQ(turnTrack.getDealedCards(), 8);
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_TRUE(turnTrack.nextTurn());
  EXPECT_FALSE(turnTrack.nextTurn());
}