#include "phase_machine_test_helper.hpp"

// 合法手が存在しないARは自動的に完了へ遷移する
TEST_F(PhaseMachineTest, ActionRoundSkipsWhenNoLegalMoves) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USA, CardEnum::DUCK_AND_COVER);

  board.pushState(StateType::AR_USSR);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(std::get<1>(result), Side::USA);
  EXPECT_FALSE(std::get<0>(result).empty());
  EXPECT_EQ(board.getActionRoundTrack().getActionRound(Side::USSR), 1);
  EXPECT_EQ(board.getActionRoundTrack().getActionRound(Side::USA), 0);
}

// USSRが追加ARを保持している場合にEXTRA_AR_USSRステートを経由して入力待ちになることを検証する
TEST_F(PhaseMachineTest, ExtraActionRoundForUssrRequestsInput) {
  board.clearHand(Side::USSR);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);

  auto& track = board.getActionRoundTrack();
  const int turn = board.getTurnTrack().getTurn();
  const int defined_rounds = track.getDefinedActionRounds(turn);

  for (int i = 0; i < defined_rounds; ++i) {
    track.advanceActionRound(Side::USSR, turn);
    track.advanceActionRound(Side::USA, turn);
  }

  track.setExtraActionRound(Side::USSR);
  track.clearExtraActionRound(Side::USA);

  board.pushState(StateType::AR_USSR_COMPLETE);

  auto result = PhaseMachine::step(board, std::nullopt);

  auto& moves = std::get<0>(result);
  EXPECT_EQ(std::get<1>(result), Side::USSR);
  EXPECT_FALSE(moves.empty());
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
  EXPECT_FALSE(track.hasExtraActionRound(Side::USSR));
  ASSERT_FALSE(board.getStates().empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(board.getStates().back()));
  EXPECT_EQ(std::get<StateType>(board.getStates().back()),
            StateType::AR_USSR_COMPLETE);
}

TEST_F(PhaseMachineTest, ExtraActionRoundOffersPassMove) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);

  auto& track = board.getActionRoundTrack();
  const int turn = board.getTurnTrack().getTurn();
  const int defined_rounds = track.getDefinedActionRounds(turn);

  for (int i = 0; i < defined_rounds; ++i) {
    track.advanceActionRound(Side::USSR, turn);
    track.advanceActionRound(Side::USA, turn);
  }

  track.setExtraActionRound(Side::USSR);
  track.clearExtraActionRound(Side::USA);

  board.pushState(StateType::AR_USSR_COMPLETE);

  auto result = PhaseMachine::step(board, std::nullopt);

  auto& moves = std::get<0>(result);
  EXPECT_EQ(std::get<1>(result), Side::USSR);
  ASSERT_FALSE(moves.empty());
  const bool has_pass =
      std::any_of(moves.begin(), moves.end(), [](const auto& move) {
        return dynamic_cast<PassMove*>(move.get()) != nullptr;
      });
  EXPECT_TRUE(has_pass);
}

// RequestCommandが合法手を返さない場合でもフェーズが前進する
TEST_F(PhaseMachineTest, RequestCommandWithNoLegalMovesIsDiscarded) {
  board.clearHand(Side::USSR);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);

  board.pushState(StateType::AR_USSR);
  board.pushState(std::make_shared<RequestCommand>(
      Side::USSR,
      [](const Board&) { return std::vector<std::shared_ptr<Move>>{}; }));

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(std::get<1>(result), Side::USSR);
  EXPECT_FALSE(std::get<0>(result).empty());
}

// RequestCommandが合法手を返した場合に入力要求が返される
TEST_F(PhaseMachineTest, RequestCommandWithLegalMovesReturnsInput) {
  auto executed = std::make_shared<bool>(false);
  auto tracking_move = std::make_shared<TrackingMove>(CardEnum::DUCK_AND_COVER,
                                                      Side::USSR, executed);

  board.pushState(std::make_shared<RequestCommand>(
      Side::USSR, [tracking_move](const Board&) {
        return std::vector<std::shared_ptr<Move>>{tracking_move};
      }));

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(std::get<1>(result), Side::USSR);
  ASSERT_EQ(std::get<0>(result).size(), 1);
  EXPECT_EQ(std::get<0>(result).front(), tracking_move);
  ASSERT_FALSE(board.getStates().empty());
  EXPECT_TRUE(std::holds_alternative<CommandPtr>(board.getStates().back()));
  auto request_ptr = std::get<CommandPtr>(board.getStates().back());
  ASSERT_NE(dynamic_cast<RequestCommand*>(request_ptr.get()), nullptr);
}

// AR_USA_COMPLETE後にUSA側の完了ハンドラがUSSRのARをスケジュールする
TEST_F(PhaseMachineTest, ArUsaCompleteSchedulesUssrActionRound) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);

  board.pushState(StateType::AR_USA_COMPLETE);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(std::get<1>(result), Side::USSR);
  EXPECT_FALSE(std::get<0>(result).empty());
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
  ASSERT_FALSE(board.getStates().empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(board.getStates().back()));
  EXPECT_EQ(std::get<StateType>(board.getStates().back()),
            StateType::AR_USSR_COMPLETE);
}

// 回答済みMoveがCommandに変換され実行されることを検証する
TEST_F(PhaseMachineTest, ProcessesAnswerExecutesCommands) {
  auto executed = std::make_shared<bool>(false);
  auto tracking_move = std::make_shared<TrackingMove>(CardEnum::DUCK_AND_COVER,
                                                      Side::USSR, executed);

  board.pushState(StateType::USSR_WIN_END);
  board.pushState(std::make_shared<RequestCommand>(
      Side::USSR, [tracking_move](const Board&) {
        return std::vector<std::shared_ptr<Move>>{tracking_move};
      }));

  std::optional<std::shared_ptr<Move>> answer = tracking_move;
  auto result = PhaseMachine::step(board, std::move(answer));

  EXPECT_TRUE(*executed);
  ASSERT_TRUE(std::get<2>(result).has_value());
  EXPECT_EQ(std::get<2>(result).value(), Side::USSR);
}

// USAの追加ARが要求された場合に対応する分岐を通る
TEST_F(PhaseMachineTest, ExtraActionRoundForUsaRequestsInput) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);

  auto& track = board.getActionRoundTrack();
  const int turn = board.getTurnTrack().getTurn();
  const int defined_rounds = track.getDefinedActionRounds(turn);

  for (int i = 0; i < defined_rounds; ++i) {
    track.advanceActionRound(Side::USSR, turn);
  }
  for (int i = 0; i < defined_rounds - 1; ++i) {
    track.advanceActionRound(Side::USA, turn);
  }

  track.clearExtraActionRound(Side::USSR);
  track.setExtraActionRound(Side::USA);

  board.pushState(StateType::AR_USA_COMPLETE);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(std::get<1>(result), Side::USA);
  EXPECT_FALSE(std::get<0>(result).empty());
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USA);
  EXPECT_FALSE(track.hasExtraActionRound(Side::USA));
  ASSERT_FALSE(board.getStates().empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(board.getStates().back()));
  EXPECT_EQ(std::get<StateType>(board.getStates().back()),
            StateType::AR_USA_COMPLETE);
}
