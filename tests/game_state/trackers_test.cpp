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
  ActionRoundTrack actionRoundTrack;
};

TEST_F(TrackTest, SpaceTrackTest) {
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 0));
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 1));
  EXPECT_TRUE(spaceTrack.canSpaceChallenge(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpaceChallenge(Side::USSR));
  spaceTrack.advanceSpaceTrack(Side::USSR, 1);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 1);
  spaceTrack.advanceSpaceTrack(Side::USSR, 1);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 2);
  EXPECT_TRUE(spaceTrack.canSpaceChallenge(Side::USSR));
  spaceTrack.spaceTried(Side::USSR);
  EXPECT_FALSE(spaceTrack.canSpaceChallenge(Side::USSR));
}

TEST_F(TrackTest, SpaceTrackCanSpaceTest) {
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 2));
  spaceTrack.advanceSpaceTrack(Side::USSR, 4);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 4);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 2));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 3));
  spaceTrack.advanceSpaceTrack(Side::USSR, 3);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 7);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 3));
  EXPECT_TRUE(spaceTrack.canSpace(Side::USSR, 4));
  spaceTrack.advanceSpaceTrack(Side::USSR, 1);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USSR), 8);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USSR, 4));
}

TEST_F(TrackTest, SpaceTrackCanSpaceEdgeCasesTest) {
  // space_index == 3の境界でopeValue < 2の場合
  spaceTrack.advanceSpaceTrack(Side::USA, 3);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USA, 1));

  // space_index == 4の境界でopeValue < 3の場合
  spaceTrack.advanceSpaceTrack(Side::USA, 1);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USA), 4);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USA, 2));

  // space_index == 6の境界でopeValue < 3の場合
  spaceTrack.advanceSpaceTrack(Side::USA, 2);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USA), 6);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USA, 2));

  // space_index == 7でopeValue < 4の場合
  spaceTrack.advanceSpaceTrack(Side::USA, 1);
  EXPECT_EQ(spaceTrack.getSpaceTrackPosition(Side::USA), 7);
  EXPECT_FALSE(spaceTrack.canSpace(Side::USA, 3));
}

TEST_F(TrackTest, DefconTrackTest) {
  defconTrack.setDefcon(4);
  EXPECT_EQ(defconTrack.getDefcon(), 4);
  // setDefconは範囲チェックを行わない（合法手判断はLegalMovesGeneratorで行う）
  defconTrack.setDefcon(0);
  EXPECT_EQ(defconTrack.getDefcon(), 0);
  defconTrack.setDefcon(6);
  EXPECT_EQ(defconTrack.getDefcon(), 6);
  // changeDefconは範囲をクランプする
  defconTrack.setDefcon(4);
  defconTrack.changeDefcon(-1);
  EXPECT_EQ(defconTrack.getDefcon(), 3);
}

TEST_F(TrackTest, DefconTrackClampTest) {
  defconTrack.setDefcon(1);
  defconTrack.changeDefcon(-1);
  EXPECT_EQ(defconTrack.getDefcon(), 1);

  defconTrack.setDefcon(5);
  defconTrack.changeDefcon(1);
  EXPECT_EQ(defconTrack.getDefcon(), 5);

  defconTrack.setDefcon(3);
  defconTrack.changeDefcon(-10);
  EXPECT_EQ(defconTrack.getDefcon(), 1);

  defconTrack.setDefcon(2);
  defconTrack.changeDefcon(10);
  EXPECT_EQ(defconTrack.getDefcon(), 5);
}

TEST_F(TrackTest, MilopsTrackTest) {
  milopsTrack.advanceMilopsTrack(Side::USSR, 2);
  EXPECT_EQ(milopsTrack.getMilops(Side::USSR), 2);
  milopsTrack.advanceMilopsTrack(Side::USA, 3);
  EXPECT_EQ(milopsTrack.getMilops(Side::USA), 3);
  milopsTrack.resetMilopsTrack();
  EXPECT_EQ(milopsTrack.getMilops(Side::USSR), 0);
  EXPECT_EQ(milopsTrack.getMilops(Side::USA), 0);
  milopsTrack.advanceMilopsTrack(Side::USSR, 7);
  EXPECT_EQ(milopsTrack.getMilops(Side::USSR), 5);
}

TEST_F(TrackTest, TurnTrackTest) {
  EXPECT_EQ(turnTrack.getTurn(), 1);
  turnTrack.nextTurn();
  EXPECT_EQ(turnTrack.getTurn(), 2);
  EXPECT_EQ(turnTrack.getDealedCards(), 8);
  turnTrack.nextTurn();
  turnTrack.nextTurn();
  turnTrack.nextTurn();
  turnTrack.nextTurn();
  turnTrack.nextTurn();
  turnTrack.nextTurn();
  turnTrack.nextTurn();
  turnTrack.nextTurn();
  turnTrack.nextTurn();
}

TEST_F(TrackTest, SpaceTrackGetRollMaxTest) {
  EXPECT_EQ(spaceTrack.getRollMax(Side::USSR), 3);
  spaceTrack.advanceSpaceTrack(Side::USSR, 1);
  EXPECT_EQ(spaceTrack.getRollMax(Side::USSR), 4);
  spaceTrack.advanceSpaceTrack(Side::USSR, 1);
  EXPECT_EQ(spaceTrack.getRollMax(Side::USSR), 3);
  spaceTrack.advanceSpaceTrack(Side::USSR, 6);
  EXPECT_EQ(spaceTrack.getRollMax(Side::USSR), 0);
}

TEST_F(TrackTest, SpaceTrackEffectEnabledTest) {
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 1));
  spaceTrack.advanceSpaceTrack(Side::USSR, 1);
  EXPECT_TRUE(spaceTrack.effectEnabled(Side::USSR, 1));
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 2));
  spaceTrack.advanceSpaceTrack(Side::USA, 1);
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 1));
  spaceTrack.advanceSpaceTrack(Side::USSR, 1);
  EXPECT_TRUE(spaceTrack.effectEnabled(Side::USSR, 2));

  // 相手側の宇宙進歩度が高い場合のテスト
  spaceTrack.advanceSpaceTrack(Side::USA, 2);
  EXPECT_FALSE(spaceTrack.effectEnabled(Side::USSR, 2));
}

TEST_F(TrackTest, SpaceTrackGetSpaceVpTest) {
  auto vp0 = SpaceTrack::getSpaceVp(0);
  EXPECT_EQ(vp0[0], 0);
  EXPECT_EQ(vp0[1], 0);

  auto vp1 = SpaceTrack::getSpaceVp(1);
  EXPECT_EQ(vp1[0], 2);
  EXPECT_EQ(vp1[1], 1);

  auto vp2 = SpaceTrack::getSpaceVp(2);
  EXPECT_EQ(vp2[0], 0);
  EXPECT_EQ(vp2[1], 0);

  auto vp3 = SpaceTrack::getSpaceVp(3);
  EXPECT_EQ(vp3[0], 2);
  EXPECT_EQ(vp3[1], 0);

  auto vp4 = SpaceTrack::getSpaceVp(4);
  EXPECT_EQ(vp4[0], 0);
  EXPECT_EQ(vp4[1], 0);

  auto vp5 = SpaceTrack::getSpaceVp(5);
  EXPECT_EQ(vp5[0], 3);
  EXPECT_EQ(vp5[1], 1);

  auto vp6 = SpaceTrack::getSpaceVp(6);
  EXPECT_EQ(vp6[0], 0);
  EXPECT_EQ(vp6[1], 0);

  auto vp7 = SpaceTrack::getSpaceVp(7);
  EXPECT_EQ(vp7[0], 4);
  EXPECT_EQ(vp7[1], 2);

  auto vp8 = SpaceTrack::getSpaceVp(8);
  EXPECT_EQ(vp8[0], 2);
  EXPECT_EQ(vp8[1], 0);

  auto vp9 = SpaceTrack::getSpaceVp(9);
  EXPECT_EQ(vp9[0], 0);
  EXPECT_EQ(vp9[1], 0);
}

TEST_F(TrackTest, ActionRoundTrackTest) {
  EXPECT_EQ(actionRoundTrack.getActionRound(Side::USSR), 0);
  EXPECT_EQ(actionRoundTrack.getActionRound(Side::USA), 0);
}

TEST_F(TrackTest, ExtraActionRoundEnablesOnlyWhenAheadAtEight) {
  // 初期状態ではどちらの陣営にも追加アクション権がないことを確認。
  actionRoundTrack.updateExtraActionRound(spaceTrack);
  EXPECT_FALSE(actionRoundTrack.hasExtraActionRound(Side::USSR));
  EXPECT_FALSE(actionRoundTrack.hasExtraActionRound(Side::USA));

  // USSRがスペーストラック8に到達し、USAが未到達ならUSSRのみ権利を得る。
  spaceTrack.advanceSpaceTrack(Side::USSR, 8);
  actionRoundTrack.updateExtraActionRound(spaceTrack);
  EXPECT_TRUE(actionRoundTrack.hasExtraActionRound(Side::USSR));
  EXPECT_FALSE(actionRoundTrack.hasExtraActionRound(Side::USA));
}

TEST_F(TrackTest, ExtraActionRoundDisablesWhenOpponentCatchesUp) {
  // USSRがリードして権利を取得した直後にUSAが追い付いた場合、双方の権利が失われる。
  spaceTrack.advanceSpaceTrack(Side::USSR, 8);
  actionRoundTrack.updateExtraActionRound(spaceTrack);
  EXPECT_TRUE(actionRoundTrack.hasExtraActionRound(Side::USSR));

  spaceTrack.advanceSpaceTrack(Side::USA, 8);
  actionRoundTrack.updateExtraActionRound(spaceTrack);
  EXPECT_FALSE(actionRoundTrack.hasExtraActionRound(Side::USSR));
  EXPECT_FALSE(actionRoundTrack.hasExtraActionRound(Side::USA));
}
