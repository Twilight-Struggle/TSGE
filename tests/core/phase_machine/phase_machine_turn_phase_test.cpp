#include "phase_machine_test_helper.hpp"

// ターン開始時に必要枚数が正しく配布されることを検証する
TEST_F(PhaseMachineTest, TurnStartDealsCardsToBothPlayers) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);
  board.addCardToHand(Side::USA, CardEnum::NUCLEAR_TEST_BAN);

  prepareDeckWithDummyCards(60);
  const int current_turn = board.getTurnTrack().getTurn();
  const auto draw_counts = board.calculateDrawCount(current_turn);
  const std::size_t initial_deck_size = board.getDeck().getDeck().size();

  board.pushState(StateType::TURN_START);

  auto result = PhaseMachine::step(board, std::nullopt);
  const auto& legal_moves = std::get<0>(result);

  EXPECT_EQ(legal_moves.size(), static_cast<std::size_t>(1 + draw_counts[0]));
  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), 1 + draw_counts[0]);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), 2 + draw_counts[1]);
  EXPECT_EQ(board.getDeck().getDeck().size(),
            initial_deck_size -
                static_cast<std::size_t>(draw_counts[0] + draw_counts[1]));
  EXPECT_EQ(std::get<1>(result), Side::USSR);
  EXPECT_TRUE(std::get<0>(result).size() > 0);
}

// TURN_ENDでターンが進み、ARカウントがリセットされることを確認する
TEST_F(PhaseMachineTest, TurnEndAdvancesTurnAndResetsActionRounds) {
  const int current_turn = board.getTurnTrack().getTurn();
  auto& action_track = board.getActionRoundTrack();
  auto& milops_track = board.getMilopsTrack();
  auto& defcon_track = board.getDefconTrack();
  for (int i = 0; i < 2; ++i) {
    action_track.advanceActionRound(Side::USSR, current_turn);
    action_track.advanceActionRound(Side::USA, current_turn);
  }

  defcon_track.setDefcon(3);
  milops_track.advanceMilopsTrack(Side::USSR, 3);
  milops_track.advanceMilopsTrack(Side::USA, 1);

  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);
  board.addCardEffectInThisTurn(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardEffectInThisTurn(Side::USA, CardEnum::FIDEL);

  board.pushState(StateType::USSR_WIN_END);
  board.pushState(StateType::TURN_END);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(board.getTurnTrack().getTurn(), current_turn + 1);
  EXPECT_EQ(action_track.getActionRound(Side::USSR), 0);
  EXPECT_EQ(action_track.getActionRound(Side::USA), 0);
  EXPECT_EQ(milops_track.getMilops(Side::USSR), 0);
  EXPECT_EQ(milops_track.getMilops(Side::USA), 0);
  EXPECT_EQ(defcon_track.getDefcon(), 4);
  EXPECT_TRUE(board.getCardsEffectsInThisTurn(Side::USSR).empty());
  EXPECT_TRUE(board.getCardsEffectsInThisTurn(Side::USA).empty());
  EXPECT_EQ(board.getVp(), 2);
  EXPECT_FALSE(std::get<0>(result).empty());
}

TEST_F(PhaseMachineTest, TurnEndRevealsChinaCard) {
  board.giveChinaCardTo(Side::USA, false);
  board.pushState(StateType::USSR_WIN_END);
  board.pushState(StateType::TURN_END);

  PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(board.getChinaCardOwner(), Side::USA);
  EXPECT_TRUE(board.isChinaCardFaceUp());
}

// MilOps差分は同時精算されるため、VPが閾値を跨いでも即時勝敗が決まらないことを確認する
TEST_F(PhaseMachineTest, TurnEndMilopsPenaltyResolvesSimultaneously) {
  const int current_turn = board.getTurnTrack().getTurn();
  auto& action_track = board.getActionRoundTrack();
  for (int i = 0; i < 2; ++i) {
    action_track.advanceActionRound(Side::USSR, current_turn);
    action_track.advanceActionRound(Side::USA, current_turn);
  }

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);

  board.changeVp(19);
  auto& defcon_track = board.getDefconTrack();
  defcon_track.setDefcon(2);
  // 両陣営ともMilOps=0のため不足量は同一。

  board.pushState(StateType::USSR_WIN_END);
  board.pushState(StateType::TURN_END);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(board.getVp(), 19);
  EXPECT_FALSE(std::get<2>(result).has_value());
}

// USSRがVP19点の状態でMilOps不足差分により勝利へ到達することを検証する
TEST_F(PhaseMachineTest, TurnEndMilopsPenaltyTriggersUssrVictory) {
  const int current_turn = board.getTurnTrack().getTurn();
  auto& action_track = board.getActionRoundTrack();
  for (int i = 0; i < 2; ++i) {
    action_track.advanceActionRound(Side::USSR, current_turn);
    action_track.advanceActionRound(Side::USA, current_turn);
  }

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);

  auto& milops_track = board.getMilopsTrack();
  auto& defcon_track = board.getDefconTrack();
  defcon_track.setDefcon(3);
  milops_track.advanceMilopsTrack(Side::USSR, 3);

  board.changeVp(19);

  board.pushState(StateType::TURN_END);

  auto result = PhaseMachine::step(board, std::nullopt);

  ASSERT_TRUE(std::get<2>(result).has_value());
  EXPECT_EQ(std::get<2>(result).value(), Side::USSR);
  EXPECT_GE(board.getVp(), 21);
}

// ターン3終了時にMid Warカードが山札へ投入されることを確認する
TEST_F(PhaseMachineTest, TurnEndAddsMidWarCardsAtTurnThree) {
  auto& pool = defaultCardPool();
  CardPoolGuard mid_guard(pool, CardEnum::DUCK_AND_COVER,
                          std::make_unique<WarPeriodTaggedCard>(
                              CardEnum::DUCK_AND_COVER, WarPeriod::MID_WAR));

  auto& deck = board.getDeck();
  deck.getDeck().clear();
  deck.getDiscardPile().clear();

  auto& turn_track = board.getTurnTrack();
  while (turn_track.getTurn() < 3) {
    turn_track.nextTurn();
  }

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  auto& action_track = board.getActionRoundTrack();
  const int next_turn = std::min(turn_track.getTurn() + 1, 10);
  const int required_cards = action_track.getDefinedActionRounds(next_turn) + 2;
  for (int i = 0; i < required_cards; ++i) {
    board.addCardToHand(Side::USSR, CardEnum::DUMMY);
    board.addCardToHand(Side::USA, CardEnum::DUMMY);
  }

  board.pushState(StateType::TURN_END);
  PhaseMachine::step(board, std::nullopt);

  const auto& cards = deck.getDeck();
  ASSERT_EQ(cards.size(), 1U);
  EXPECT_EQ(cards.front(), CardEnum::DUCK_AND_COVER);
}

// ターン7終了時にLate Warカードが山札へ投入されることを確認する
TEST_F(PhaseMachineTest, TurnEndAddsLateWarCardsAtTurnSeven) {
  auto& pool = defaultCardPool();
  CardPoolGuard late_guard(pool, CardEnum::FIDEL,
                           std::make_unique<WarPeriodTaggedCard>(
                               CardEnum::FIDEL, WarPeriod::LATE_WAR));

  auto& deck = board.getDeck();
  deck.getDeck().clear();
  deck.getDiscardPile().clear();

  auto& turn_track = board.getTurnTrack();
  while (turn_track.getTurn() < 7) {
    turn_track.nextTurn();
  }

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  auto& action_track = board.getActionRoundTrack();
  const int next_turn = std::min(turn_track.getTurn() + 1, 10);
  const int required_cards = action_track.getDefinedActionRounds(next_turn) + 2;
  for (int i = 0; i < required_cards; ++i) {
    board.addCardToHand(Side::USSR, CardEnum::DUMMY);
    board.addCardToHand(Side::USA, CardEnum::DUMMY);
  }

  board.pushState(StateType::TURN_END);
  PhaseMachine::step(board, std::nullopt);

  const auto& cards = deck.getDeck();
  ASSERT_EQ(cards.size(), 1U);
  EXPECT_EQ(cards.front(), CardEnum::FIDEL);
}
