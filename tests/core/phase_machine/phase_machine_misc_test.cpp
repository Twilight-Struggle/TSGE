#include "phase_machine_test_helper.hpp"

TEST_F(PhaseMachineTest, BoardArPlayerFunctionalityTest) {
  // 初期状態確認
  EXPECT_EQ(board.getCurrentArPlayer(), Side::NEUTRAL);

  // ARプレイヤーの設定テスト
  board.setCurrentArPlayer(Side::USSR);
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);

  board.setCurrentArPlayer(Side::USA);
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USA);

  // NEUTRALに戻す
  board.setCurrentArPlayer(Side::NEUTRAL);
  EXPECT_EQ(board.getCurrentArPlayer(), Side::NEUTRAL);
}

// 終端状態を検出できることを確認する
TEST_F(PhaseMachineTest, TerminalStateReturnsWinner) {
  board.pushState(StateType::USSR_WIN_END);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_TRUE(std::get<0>(result).empty());
  EXPECT_EQ(std::get<1>(result), Side::NEUTRAL);
  ASSERT_TRUE(std::get<2>(result).has_value());
  EXPECT_EQ(std::get<2>(result).value(), Side::USSR);
}

TEST_F(PhaseMachineTest, TurnEndRequestsSpaceTrackDiscardWhenAdvantaged) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::FIDEL);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::NUCLEAR_TEST_BAN);

  prepareDeckWithDummyCards(60);

  board.getSpaceTrack().advanceSpaceTrack(Side::USSR, 6);
  board.getSpaceTrack().advanceSpaceTrack(Side::USA, 3);

  auto& milops_track = board.getMilopsTrack();
  milops_track.advanceMilopsTrack(Side::USSR, 5);
  milops_track.advanceMilopsTrack(Side::USA, 5);

  board.pushState(StateType::TURN_END);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(std::get<1>(result), Side::USSR);
  const auto& moves = std::get<0>(result);
  const auto pass_count =
      std::count_if(moves.begin(), moves.end(), [](const auto& move) {
        return dynamic_cast<PassMove*>(move.get()) != nullptr;
      });
  EXPECT_EQ(pass_count, 1);
  const auto discard_count =
      std::count_if(moves.begin(), moves.end(), [](const auto& move) {
        return dynamic_cast<DiscardMove*>(move.get()) != nullptr;
      });
  EXPECT_GE(discard_count, 2);
  EXPECT_TRUE(std::any_of(moves.begin(), moves.end(), [](const auto& move) {
    const auto* discard = dynamic_cast<DiscardMove*>(move.get());
    return discard != nullptr && discard->getCard() == CardEnum::FIDEL;
  }));
  EXPECT_TRUE(std::any_of(moves.begin(), moves.end(), [](const auto& move) {
    const auto* discard = dynamic_cast<DiscardMove*>(move.get());
    return discard != nullptr && discard->getCard() == CardEnum::DUCK_AND_COVER;
  }));
}

TEST_F(PhaseMachineTest, SpaceTrackDiscardMoveRemovesCardAndResumesTurn) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::FIDEL);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::NUCLEAR_TEST_BAN);

  prepareDeckWithDummyCards(60);
  const int current_turn = board.getTurnTrack().getTurn();
  const int next_turn = std::min(current_turn + 1, 10);
  const int required_cards =
      board.getActionRoundTrack().getDefinedActionRounds(next_turn) + 2;
  const int initial_ussr_hand =
      static_cast<int>(board.getPlayerHand(Side::USSR).size());
  const int expected_ussr_after_draw =
      std::max(initial_ussr_hand, required_cards);

  board.getSpaceTrack().advanceSpaceTrack(Side::USSR, 6);
  board.getSpaceTrack().advanceSpaceTrack(Side::USA, 3);

  auto& milops_track = board.getMilopsTrack();
  milops_track.advanceMilopsTrack(Side::USSR, 5);
  milops_track.advanceMilopsTrack(Side::USA, 5);

  board.pushState(StateType::TURN_END);

  auto initial_result = PhaseMachine::step(board, std::nullopt);
  auto moves = std::get<0>(initial_result);
  ASSERT_EQ(moves.size(), 3U);

  auto discard_it =
      std::find_if(moves.begin(), moves.end(), [](const auto& mv) {
        return dynamic_cast<DiscardMove*>(mv.get()) != nullptr;
      });
  ASSERT_NE(discard_it, moves.end());
  const CardEnum discarded_card = (*discard_it)->getCard();

  std::optional<std::shared_ptr<Move>> answer = *discard_it;
  auto next_result = PhaseMachine::step(board, std::move(answer));

  const auto& ussr_hand = board.getPlayerHand(Side::USSR);
  EXPECT_EQ(ussr_hand.size(),
            static_cast<std::size_t>(expected_ussr_after_draw));
  EXPECT_TRUE(std::none_of(
      ussr_hand.begin(), ussr_hand.end(),
      [discarded_card](CardEnum card) { return card == discarded_card; }));

  const auto& discard_pile = board.getDeck().getDiscardPile();
  ASSERT_EQ(discard_pile.size(), 1U);
  EXPECT_EQ(discard_pile.front(), discarded_card);

  EXPECT_EQ(std::get<1>(next_result), Side::USA);
  EXPECT_FALSE(std::get<0>(next_result).empty());
}
