// File: tests/game_state/special_cards_test.cpp
// Summary: 特殊な影響力配置カードのイベント実装を検証する。
// Reason: カードイベントがRequestCommandを正しく生成するかを保証する。

#include <gtest/gtest.h>

#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

class DeStainizationSpecialCardTest : public ::testing::Test {
 protected:
  DeStainizationSpecialCardTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }

  Board board;
  DeStainization sut;
};

TEST_F(DeStainizationSpecialCardTest, GeneratesRequestCommand) {
  EXPECT_TRUE(sut.canEvent(board));

  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}
