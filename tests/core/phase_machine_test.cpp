#include "tsge/core/phase_machine.hpp"

#include <gtest/gtest.h>

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

class PhaseMachineTest : public ::testing::Test {
 protected:
  PhaseMachineTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    if (!pool[0]) {
      // Create a dummy card for testing
      pool[0] = std::make_unique<DummyCard>();
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
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);
  board.addCardToHand(Side::USA, CardEnum::Fidel);

  // TURN_STARTをプッシュしてヘッドラインフェイズをトリガー
  board.pushState(StateType::TURN_START);

  // TURN_STARTを実行してHEADLINE_PHASEがプッシュされることを確認
  auto result = PhaseMachine::step(board, std::nullopt);

  // HEADLINE_CARD_SELECT_USSRが返されることを期待（同時選択の疑似実装）
  EXPECT_EQ(std::get<1>(result), Side::USSR);
  EXPECT_FALSE(std::get<0>(result).empty());
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