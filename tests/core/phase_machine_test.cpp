#include "tsge/core/phase_machine.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

//===----------------------------------------------------------------------===//
// File: tests/core/phase_machine_test.cpp
// 内容: PhaseMachineの状態遷移とヘッドライン処理を含む主要フローを検証する。
// 目的: ボード進行の安全性を担保し、回帰を早期に検出する。
//===----------------------------------------------------------------------===//

#include "tsge/actions/command.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"

// Dummy card class for testing
class DummyCard : public Card {
 public:
  DummyCard()
      : Card(CardEnum::Dummy, "Dummy", 3, Side::NEUTRAL, WarPeriod::DUMMY,
             false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    return {};
  }
  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return true;
  }
};

class HeadlineEventCard final : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  HeadlineEventCard(CardEnum id, std::string&& name, int ops, Side allegiance,
                    std::shared_ptr<std::vector<std::pair<Side, CardEnum>>> log,
                    bool removedAfterEvent = false, bool canEventValue = true)
      : Card(id, std::move(name), ops, allegiance, WarPeriod::DUMMY,
             removedAfterEvent),
        executionLog_(std::move(log)),
        canEventEnabled_(canEventValue) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    auto log = executionLog_;
    const auto card_id = id_;
    return {std::make_shared<LambdaCommand>(
        [log, side, card_id](Board&) { log->emplace_back(side, card_id); })};
  }

  [[nodiscard]]
  bool canEvent(const Board& /*board*/) const override {
    return canEventEnabled_;
  }

 private:
  std::shared_ptr<std::vector<std::pair<Side, CardEnum>>> executionLog_;
  bool canEventEnabled_;
};

class CardPoolGuard final {
 public:
  CardPoolGuard(std::array<std::unique_ptr<Card>, 111>& pool, CardEnum card,
                std::unique_ptr<Card> replacement)
      : pool_(pool),
        index_(static_cast<std::size_t>(card)),
        original_(std::move(pool_[index_])) {
    pool_[index_] = std::move(replacement);
  }

  CardPoolGuard(const CardPoolGuard&) = delete;
  CardPoolGuard& operator=(const CardPoolGuard&) = delete;

  CardPoolGuard(CardPoolGuard&&) = delete;
  CardPoolGuard& operator=(CardPoolGuard&&) = delete;

  ~CardPoolGuard() { pool_[index_] = std::move(original_); }

 private:
  std::array<std::unique_ptr<Card>, 111>& pool_;
  std::size_t index_;
  std::unique_ptr<Card> original_;
};

class TrackingMove final : public Move {
 public:
  TrackingMove(CardEnum cardEnum, Side side, std::shared_ptr<bool> executed)
      : Move(cardEnum, side), executedFlag_(std::move(executed)) {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override {
    auto flag = executedFlag_;
    return {std::make_shared<LambdaCommand>([flag](Board&) { *flag = true; })};
  }

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast = dynamic_cast<const TrackingMove*>(&other);
    return other_cast != nullptr && other_cast->executedFlag_ == executedFlag_;
  }

 private:
  std::shared_ptr<bool> executedFlag_;
};

class PhaseMachineTest : public ::testing::Test {
 protected:
  PhaseMachineTest() : board(defaultCardPool()) {}

  static std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    static bool initialized = false;
    if (!initialized) {
      for (auto& card : pool) {
        if (!card) {
          card = std::make_unique<DummyCard>();
        }
      }
      initialized = true;
    }
    return pool;
  }

  Board board;
};

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

// ヘッドラインフェイズの基本テスト
TEST_F(PhaseMachineTest, HeadlinePhaseBasicFlow) {
  // プレイヤーに手札を追加
  auto execution_log =
      std::make_shared<std::vector<std::pair<Side, CardEnum>>>();
  auto& pool = defaultCardPool();
  CardPoolGuard ussr_guard(pool, CardEnum::DuckAndCover,
                           std::make_unique<HeadlineEventCard>(
                               CardEnum::DuckAndCover, "USSR Headline", 2,
                               Side::USSR, execution_log));
  CardPoolGuard usa_guard(
      pool, CardEnum::Fidel,
      std::make_unique<HeadlineEventCard>(CardEnum::Fidel, "USA Headline", 4,
                                          Side::USA, execution_log));

  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);
  board.addCardToHand(Side::USSR, CardEnum::NuclearTestBan);
  board.addCardToHand(Side::USA, CardEnum::Fidel);
  board.addCardToHand(Side::USA, CardEnum::NuclearTestBan);

  // TURN_STARTをプッシュしてヘッドラインフェイズをトリガー
  board.pushState(StateType::TURN_START);

  // TURN_STARTを実行してHEADLINE_PHASEがプッシュされることを確認
  auto first_result = PhaseMachine::step(board, std::nullopt);

  auto& ussr_moves = std::get<0>(first_result);
  ASSERT_EQ(ussr_moves.size(), 2);
  auto ussr_move_it =
      std::find_if(ussr_moves.begin(), ussr_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::DuckAndCover;
      });
  ASSERT_NE(ussr_move_it, ussr_moves.end());
  auto ussr_move = *ussr_move_it;

  // HEADLINE_CARD_SELECT_USSRが返されることを期待（同時選択の疑似実装）
  EXPECT_EQ(std::get<1>(first_result), Side::USSR);
  EXPECT_TRUE(ussr_move);

  auto second_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(ussr_move)));

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), 1);
  EXPECT_EQ(board.getHeadlineCard(Side::USSR), CardEnum::DuckAndCover);

  auto& usa_moves = std::get<0>(second_result);
  ASSERT_EQ(usa_moves.size(), 2);
  auto usa_move_it = std::find_if(
      usa_moves.begin(), usa_moves.end(),
      [](const auto& move) { return move->getCard() == CardEnum::Fidel; });
  ASSERT_NE(usa_move_it, usa_moves.end());
  auto usa_move = *usa_move_it;
  EXPECT_EQ(std::get<1>(second_result), Side::USA);
  EXPECT_TRUE(usa_move);

  auto third_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(usa_move)));

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), 1);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), 1);
  EXPECT_EQ(std::get<1>(third_result), Side::USSR);
  const auto& ar_moves = std::get<0>(third_result);
  ASSERT_FALSE(ar_moves.empty());
  EXPECT_TRUE(
      std::any_of(ar_moves.begin(), ar_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::NuclearTestBan &&
               move->getSide() == Side::USSR;
      }));
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
  EXPECT_EQ(board.getHeadlineCard(Side::USSR), CardEnum::Dummy);
  EXPECT_EQ(board.getHeadlineCard(Side::USA), CardEnum::Dummy);

  const std::vector<std::pair<Side, CardEnum>> expected_log = {
      {Side::USA, CardEnum::Fidel}, {Side::USSR, CardEnum::DuckAndCover}};
  ASSERT_EQ(execution_log->size(), expected_log.size());
  EXPECT_EQ(*execution_log, expected_log);

  for (const auto& state_variant : board.getStates()) {
    if (std::holds_alternative<CommandPtr>(state_variant)) {
      const auto& command = std::get<CommandPtr>(state_variant);
      EXPECT_EQ(dynamic_cast<LambdaCommand*>(command.get()), nullptr);
    }
  }
}

// ヘッドラインで発動したカードが適切な山へ移動することを検証する
TEST_F(PhaseMachineTest, HeadlineCardsMoveToCorrectPiles) {
  auto execution_log =
      std::make_shared<std::vector<std::pair<Side, CardEnum>>>();
  auto& pool = defaultCardPool();
  CardPoolGuard ussr_guard(pool, CardEnum::DuckAndCover,
                           std::make_unique<HeadlineEventCard>(
                               CardEnum::DuckAndCover, "USSR Headline", 2,
                               Side::USSR, execution_log, false));
  CardPoolGuard usa_guard(
      pool, CardEnum::Fidel,
      std::make_unique<HeadlineEventCard>(CardEnum::Fidel, "USA Headline", 4,
                                          Side::USA, execution_log, true));

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);
  board.addCardToHand(Side::USSR, CardEnum::NuclearTestBan);
  board.addCardToHand(Side::USA, CardEnum::Fidel);
  board.addCardToHand(Side::USA, CardEnum::NuclearTestBan);

  board.pushState(StateType::TURN_START);

  auto first_result = PhaseMachine::step(board, std::nullopt);
  auto& ussr_moves = std::get<0>(first_result);
  ASSERT_EQ(ussr_moves.size(), 2);
  auto ussr_move_it =
      std::find_if(ussr_moves.begin(), ussr_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::DuckAndCover;
      });
  ASSERT_NE(ussr_move_it, ussr_moves.end());
  auto ussr_move = *ussr_move_it;

  auto second_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(ussr_move)));
  auto& usa_moves = std::get<0>(second_result);
  ASSERT_EQ(usa_moves.size(), 2);
  auto usa_move_it = std::find_if(
      usa_moves.begin(), usa_moves.end(),
      [](const auto& move) { return move->getCard() == CardEnum::Fidel; });
  ASSERT_NE(usa_move_it, usa_moves.end());
  auto usa_move = *usa_move_it;

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), 1);
  EXPECT_EQ(board.getHeadlineCard(Side::USSR), CardEnum::DuckAndCover);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), 2);

  auto third_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(usa_move)));

  // ヘッドライン処理後も各プレイヤーに1枚ずつ手札が残り、ARへ移行できる
  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), 1);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), 1);

  const auto& discard = board.getDeck().getDiscardPile();
  ASSERT_EQ(discard.size(), 1);
  EXPECT_EQ(discard.front(), CardEnum::DuckAndCover);

  const auto& removed = board.getDeck().getRemovedCards();
  ASSERT_EQ(removed.size(), 1);
  EXPECT_EQ(removed.front(), CardEnum::Fidel);

  // AR入力を要求する状態へ遷移し、USSRが行動待ちになる
  EXPECT_EQ(std::get<1>(third_result), Side::USSR);
  const auto& ar_moves_for_guard = std::get<0>(third_result);
  ASSERT_FALSE(ar_moves_for_guard.empty());
  EXPECT_TRUE(std::any_of(ar_moves_for_guard.begin(), ar_moves_for_guard.end(),
                          [](const auto& move) {
                            return move->getCard() ==
                                       CardEnum::NuclearTestBan &&
                                   move->getSide() == Side::USSR;
                          }));
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
}

// 発動不可カードでも解決後に正しく捨て札へ移動することを確認する
TEST_F(PhaseMachineTest, HeadlineCardsWithoutEventGoToDiscard) {
  auto execution_log =
      std::make_shared<std::vector<std::pair<Side, CardEnum>>>();
  auto& pool = defaultCardPool();
  CardPoolGuard ussr_guard(pool, CardEnum::DuckAndCover,
                           std::make_unique<HeadlineEventCard>(
                               CardEnum::DuckAndCover, "USSR Headline", 2,
                               Side::USSR, execution_log, true, false));
  CardPoolGuard usa_guard(pool, CardEnum::Fidel,
                          std::make_unique<HeadlineEventCard>(
                              CardEnum::Fidel, "USA Headline", 4, Side::USA,
                              execution_log, false, true));

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);
  board.addCardToHand(Side::USSR, CardEnum::NuclearTestBan);
  board.addCardToHand(Side::USA, CardEnum::Fidel);
  board.addCardToHand(Side::USA, CardEnum::NuclearTestBan);

  board.pushState(StateType::TURN_START);

  auto first_result = PhaseMachine::step(board, std::nullopt);
  auto& ussr_moves = std::get<0>(first_result);
  ASSERT_EQ(ussr_moves.size(), 2);
  auto ussr_move_it =
      std::find_if(ussr_moves.begin(), ussr_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::DuckAndCover;
      });
  ASSERT_NE(ussr_move_it, ussr_moves.end());
  auto ussr_move = *ussr_move_it;

  auto second_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(ussr_move)));
  auto& usa_moves = std::get<0>(second_result);
  ASSERT_EQ(usa_moves.size(), 2);
  auto usa_move_it = std::find_if(
      usa_moves.begin(), usa_moves.end(),
      [](const auto& move) { return move->getCard() == CardEnum::Fidel; });
  ASSERT_NE(usa_move_it, usa_moves.end());
  auto usa_move = *usa_move_it;

  auto third_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(usa_move)));

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), 1);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), 1);

  const auto& discard = board.getDeck().getDiscardPile();
  ASSERT_EQ(discard.size(), 2);
  EXPECT_EQ(std::count(discard.begin(), discard.end(), CardEnum::DuckAndCover),
            1);
  EXPECT_EQ(std::count(discard.begin(), discard.end(), CardEnum::Fidel), 1);

  const auto& removed = board.getDeck().getRemovedCards();
  EXPECT_TRUE(removed.empty());

  // USSRカードは発動していないためログにはUSAカードのみ記録される
  ASSERT_EQ(execution_log->size(), 1);
  EXPECT_EQ(execution_log->front(), std::make_pair(Side::USA, CardEnum::Fidel));

  EXPECT_EQ(std::get<1>(third_result), Side::USSR);
  const auto& ar_moves_without_event = std::get<0>(third_result);
  ASSERT_FALSE(ar_moves_without_event.empty());
  EXPECT_TRUE(std::any_of(ar_moves_without_event.begin(),
                          ar_moves_without_event.end(), [](const auto& move) {
                            return move->getCard() ==
                                       CardEnum::NuclearTestBan &&
                                   move->getSide() == Side::USSR;
                          }));
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
}

// 宇宙開発トラック優位性があるケースのテスト
TEST_F(PhaseMachineTest, HeadlinePhaseSpaceAdvantage) {
  // プレイヤーに手札を追加
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);
  board.addCardToHand(Side::USA, CardEnum::Fidel);

  // USSRが宇宙開発トラック4に到達
  board.getSpaceTrack().advanceSpaceTrack(Side::USSR, 4);

  // HEADLINE_PHASEを直接実行
  board.pushState(StateType::HEADLINE_PHASE);

  auto result = PhaseMachine::step(board, std::nullopt);

  // USAが先に選択することを期待（劣位側が先）
  EXPECT_EQ(std::get<1>(result), Side::USA);
  EXPECT_FALSE(std::get<0>(result).empty());
}

// 合法手が存在しないARは自動的に完了へ遷移する
TEST_F(PhaseMachineTest, ActionRoundSkipsWhenNoLegalMoves) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USA, CardEnum::DuckAndCover);

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
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);

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

// RequestCommandが合法手を返さない場合でもフェーズが前進する
TEST_F(PhaseMachineTest, RequestCommandWithNoLegalMovesIsDiscarded) {
  board.clearHand(Side::USSR);
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);

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
  auto tracking_move = std::make_shared<TrackingMove>(CardEnum::DuckAndCover,
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

// 終端状態を検出できることを確認する
TEST_F(PhaseMachineTest, TerminalStateReturnsWinner) {
  board.pushState(StateType::USSR_WIN_END);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_TRUE(std::get<0>(result).empty());
  EXPECT_EQ(std::get<1>(result), Side::NEUTRAL);
  ASSERT_TRUE(std::get<2>(result).has_value());
  EXPECT_EQ(std::get<2>(result).value(), Side::USSR);
}

// AR_USA_COMPLETE後にUSA側の完了ハンドラがUSSRのARをスケジュールする
TEST_F(PhaseMachineTest, ArUsaCompleteSchedulesUssrActionRound) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);
  board.addCardToHand(Side::USA, CardEnum::Fidel);

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
  auto tracking_move = std::make_shared<TrackingMove>(CardEnum::DuckAndCover,
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

// TURN_ENDでターンが進み、ARカウントがリセットされることを確認する
TEST_F(PhaseMachineTest, TurnEndAdvancesTurnAndResetsActionRounds) {
  const int current_turn = board.getTurnTrack().getTurn();
  auto& action_track = board.getActionRoundTrack();
  for (int i = 0; i < 2; ++i) {
    action_track.advanceActionRound(Side::USSR, current_turn);
    action_track.advanceActionRound(Side::USA, current_turn);
  }

  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);
  board.addCardToHand(Side::USA, CardEnum::Fidel);

  board.pushState(StateType::USSR_WIN_END);
  board.pushState(StateType::TURN_END);

  auto result = PhaseMachine::step(board, std::nullopt);

  EXPECT_EQ(board.getTurnTrack().getTurn(), current_turn + 1);
  EXPECT_EQ(action_track.getActionRound(Side::USSR), 0);
  EXPECT_EQ(action_track.getActionRound(Side::USA), 0);
  EXPECT_FALSE(std::get<0>(result).empty());
}

// USAの追加ARが要求された場合に対応する分岐を通る
TEST_F(PhaseMachineTest, ExtraActionRoundForUsaRequestsInput) {
  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USA, CardEnum::Fidel);

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
