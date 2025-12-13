#include "test_helper.hpp"
#include "tsge/actions/game_logic_legal_moves_generator.hpp"

class ActionLegalMovesForCardTest : public ::testing::Test {
 protected:
  ActionLegalMovesForCardTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionLegalMovesForCardTest, AggregatesPlaceRealignCoupMoves) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});
  board.getDefconTrack().setDefcon(5);

  auto place_moves =
      GameLogicLegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
          board, Side::USSR, CardEnum::DUCK_AND_COVER);
  auto realign_moves =
      GameLogicLegalMovesGenerator::actionRealignmentLegalMovesForCard(
          board, Side::USSR, CardEnum::DUCK_AND_COVER);
  auto coup_moves = GameLogicLegalMovesGenerator::actionCoupLegalMovesForCard(
      board, Side::USSR, CardEnum::DUCK_AND_COVER);

  auto ops_moves = GameLogicLegalMovesGenerator::actionLegalMovesForCard(
      board, Side::USSR, CardEnum::DUCK_AND_COVER);

  const size_t expected_size =
      place_moves.size() + realign_moves.size() + coup_moves.size();
  EXPECT_EQ(ops_moves.size(), expected_size);

  for (const auto& move : ops_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::DUCK_AND_COVER);
    EXPECT_EQ(move->getSide(), Side::USSR);

    const bool is_place =
        dynamic_cast<ActionPlaceInfluenceMove*>(move.get()) != nullptr;
    const bool is_realign =
        dynamic_cast<ActionRealigmentMove*>(move.get()) != nullptr;
    const bool is_coup = dynamic_cast<ActionCoupMove*>(move.get()) != nullptr;
    EXPECT_TRUE(is_place || is_realign || is_coup);
  }
}

class ExtraActionRoundLegalMovesTest : public ::testing::Test {
 protected:
  ExtraActionRoundLegalMovesTest() : board(createTestCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USSR, false); }

  Board board;
};

TEST_F(ExtraActionRoundLegalMovesTest, ProvidesPassWhenNoOtherMoves) {
  board.clearHand(Side::USSR);
  ASSERT_FALSE(board.isChinaCardAvailableFor(Side::USSR));
  EXPECT_TRUE(board.getPlayerHand(Side::USSR).empty());

  auto moves = GameLogicLegalMovesGenerator::extraActionRoundLegalMoves(
      board, Side::USSR);

  ASSERT_EQ(moves.size(), 1);
  EXPECT_NE(dynamic_cast<PassMove*>(moves.front().get()), nullptr);
}

TEST_F(ExtraActionRoundLegalMovesTest, PassIncludedAlongsideOpsMoves) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});

  auto moves = GameLogicLegalMovesGenerator::extraActionRoundLegalMoves(
      board, Side::USSR);

  ASSERT_GE(moves.size(), 2);
  const auto pass_count =
      std::count_if(moves.begin(), moves.end(), [](const auto& move) {
        return dynamic_cast<PassMove*>(move.get()) != nullptr;
      });
  EXPECT_EQ(pass_count, 1);
}

class SpaceTrackDiscardLegalMovesTest : public ::testing::Test {
 protected:
  SpaceTrackDiscardLegalMovesTest() : board(createTestCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USSR, false); }

  Board board;
};

TEST_F(SpaceTrackDiscardLegalMovesTest, ListsCardsAndPassMove) {
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::FIDEL, CardEnum::DUCK_AND_COVER});

  auto moves = GameLogicLegalMovesGenerator::spaceTrackDiscardLegalMoves(
      board, Side::USSR);

  ASSERT_EQ(moves.size(), 3);
  int pass_count = 0;
  std::vector<CardEnum> discard_candidates;
  discard_candidates.reserve(2);
  for (const auto& move : moves) {
    if (dynamic_cast<PassMove*>(move.get()) != nullptr) {
      ++pass_count;
      continue;
    }
    auto* discard = dynamic_cast<DiscardMove*>(move.get());
    ASSERT_NE(discard, nullptr);
    discard_candidates.push_back(discard->getCard());
  }

  EXPECT_EQ(pass_count, 1);
  ASSERT_EQ(discard_candidates.size(), 2U);
  EXPECT_NE(std::find(discard_candidates.begin(), discard_candidates.end(),
                      CardEnum::FIDEL),
            discard_candidates.end());
  EXPECT_NE(std::find(discard_candidates.begin(), discard_candidates.end(),
                      CardEnum::DUCK_AND_COVER),
            discard_candidates.end());
}

TEST_F(SpaceTrackDiscardLegalMovesTest, ReturnsEmptyWhenHandEmpty) {
  board.clearHand(Side::USSR);

  auto moves = GameLogicLegalMovesGenerator::spaceTrackDiscardLegalMoves(
      board, Side::USSR);

  EXPECT_TRUE(moves.empty());
}
