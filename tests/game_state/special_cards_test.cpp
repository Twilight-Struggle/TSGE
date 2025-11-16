// File: tests/game_state/special_cards_test.cpp
// Summary: 特殊な影響力配置カードのイベント実装を検証する。
// Reason: カードイベントがRequestCommandを正しく生成するかを保証する。

#include <gtest/gtest.h>

#include "tsge/actions/command.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

class SpecialCardsTest : public ::testing::Test {
 protected:
  SpecialCardsTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }

  Board board;
};

TEST_F(SpecialCardsTest, ComeconEventTest) {
  Comecon comecon;

  // canEventのテスト
  EXPECT_TRUE(comecon.canEvent(board));

  // eventコマンドの生成テスト
  auto commands = comecon.event(Side::USSR);
  EXPECT_EQ(commands.size(), 1);

  // RequestCommandが生成されていることを確認
  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, DecolonizationEventTest) {
  Decolonization decolonization;

  EXPECT_TRUE(decolonization.canEvent(board));

  auto commands = decolonization.event(Side::USSR);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, PuppetGovernmentsEventTest) {
  PuppetGovernments puppet_governments;

  EXPECT_TRUE(puppet_governments.canEvent(board));

  auto commands = puppet_governments.event(Side::USA);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, MarshallPlanEventTest) {
  MarshallPlan marshall_plan;

  EXPECT_TRUE(marshall_plan.canEvent(board));

  auto commands = marshall_plan.event(Side::USA);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, LiberationTheologyEventTest) {
  LiberationTheology liberation_theology;

  EXPECT_TRUE(liberation_theology.canEvent(board));

  auto commands = liberation_theology.event(Side::USSR);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, WarsawPactFormedEventTest) {
  WarsawPactFormed warsaw_pact;

  EXPECT_TRUE(warsaw_pact.canEvent(board));

  auto commands = warsaw_pact.event(Side::USSR);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, OASFoundedEventTest) {
  OASFounded oas_founded;

  EXPECT_TRUE(oas_founded.canEvent(board));

  auto commands = oas_founded.event(Side::USA);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, DestaLinizationEventTest) {
  DestaLinization destalinization;

  EXPECT_TRUE(destalinization.canEvent(board));

  auto commands = destalinization.event(Side::USSR);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, ColonialRearGuardsEventTest) {
  ColonialRearGuards colonial_rear_guards;

  EXPECT_TRUE(colonial_rear_guards.canEvent(board));

  auto commands = colonial_rear_guards.event(Side::USA);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, UssuriRiverSkirmishEventTest) {
  UssuriRiverSkirmish ussuri_river_skirmish;

  EXPECT_TRUE(ussuri_river_skirmish.canEvent(board));

  auto commands = ussuri_river_skirmish.event(Side::USA);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, TheReformerEventTest) {
  TheReformer the_reformer;

  EXPECT_TRUE(the_reformer.canEvent(board));

  auto commands = the_reformer.event(Side::USSR);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}