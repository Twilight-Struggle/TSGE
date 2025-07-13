#include <gtest/gtest.h>

#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"

// Dummy card class for testing
class DummyCard : public Card {
 public:
  DummyCard() : Card(0, "Dummy", 3, Side::NEUTRAL, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    return {};
  }
  bool canEvent(Board& board) const override { return true; }
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