// File: tests/game_state/special_cards_test.cpp
// Summary: 特殊な影響力配置カードのイベント実装を検証する。
// Reason: カードイベントがRequestCommandを正しく生成するかを保証する。

#include <gtest/gtest.h>

#include <set>

#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
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
  auto commands = comecon.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  // RequestCommandが生成されていることを確認
  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, DecolonizationEventTest) {
  Decolonization decolonization;

  EXPECT_TRUE(decolonization.canEvent(board));

  auto commands = decolonization.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, PuppetGovernmentsEventTest) {
  PuppetGovernments puppet_governments;

  EXPECT_TRUE(puppet_governments.canEvent(board));

  auto commands = puppet_governments.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, MarshallPlanEventTest) {
  MarshallPlan marshall_plan;

  EXPECT_TRUE(marshall_plan.canEvent(board));

  auto commands = marshall_plan.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, LiberationTheologyEventTest) {
  LiberationTheology liberation_theology;

  EXPECT_TRUE(liberation_theology.canEvent(board));

  auto commands = liberation_theology.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, WarsawPactFormedEventTest) {
  WarsawPactFormed warsaw_pact;

  EXPECT_TRUE(warsaw_pact.canEvent(board));

  auto commands = warsaw_pact.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, OASFoundedEventTest) {
  OASFounded oas_founded;

  EXPECT_TRUE(oas_founded.canEvent(board));

  auto commands = oas_founded.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, DestaLinizationEventTest) {
  DeStainization destalinization;

  EXPECT_TRUE(destalinization.canEvent(board));

  auto commands = destalinization.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, ColonialRearGuardsEventTest) {
  ColonialRearGuards colonial_rear_guards;

  EXPECT_TRUE(colonial_rear_guards.canEvent(board));

  auto commands = colonial_rear_guards.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, UssuriRiverSkirmishEventTest) {
  UssuriRiverSkirmish ussuri_river_skirmish;

  EXPECT_TRUE(ussuri_river_skirmish.canEvent(board));

  auto commands = ussuri_river_skirmish.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, TheReformerEventTest) {
  TheReformer the_reformer;

  EXPECT_TRUE(the_reformer.canEvent(board));

  auto commands = the_reformer.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SpecialCardsTest, SpecialRelationshipEventTest) {
  SpecialRelationship special_relationship;

  EXPECT_TRUE(special_relationship.canEvent(board));

  // UK支配の状態を設定
  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 5);

  // ケース1: NATO無効時（UK隣接国に+1、VP+2なし）
  {
    auto commands = special_relationship.event(Side::USA, board);
    EXPECT_EQ(commands.size(), 1);  // RequestCommandのみ

    const auto* request_cmd =
        dynamic_cast<const RequestCommand*>(commands[0].get());
    ASSERT_NE(request_cmd, nullptr);

    auto moves = request_cmd->legalMoves(board);
    EXPECT_GT(moves.size(), 0);  // UK隣接国が存在する

    // 各Moveが正しい型であることを確認
    for (const auto& move : moves) {
      const auto* event_move =
          dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
      EXPECT_NE(event_move, nullptr);
    }
  }

  // ケース2: NATO有効時（西欧に+2 + VP+2）
  {
    board.addCardEffectInProgress(CardEnum::NATO);
    auto commands = special_relationship.event(Side::USA, board);
    EXPECT_EQ(commands.size(), 2);  // RequestCommand + ChangeVpCommand

    const auto* request_cmd =
        dynamic_cast<const RequestCommand*>(commands[0].get());
    ASSERT_NE(request_cmd, nullptr);

    auto moves = request_cmd->legalMoves(board);
    EXPECT_GT(moves.size(), 0);  // 西欧の国が存在する

    // 各Moveが正しい型であることを確認
    for (const auto& move : moves) {
      const auto* event_move =
          dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
      EXPECT_NE(event_move, nullptr);
    }

    // ChangeVpCommandを確認
    const auto* vp_cmd =
        dynamic_cast<const ChangeVpCommand*>(commands[1].get());
    ASSERT_NE(vp_cmd, nullptr);
  }
}

TEST_F(SpecialCardsTest, SouthAfricanUnrestEventTest) {
  SouthAfricanUnrest south_african_unrest;

  EXPECT_TRUE(south_african_unrest.canEvent(board));

  auto commands = south_african_unrest.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  ASSERT_NE(request_cmd, nullptr);

  // RequestCommandからMoveを取得
  auto moves = request_cmd->legalMoves(board);
  EXPECT_EQ(moves.size(), 4);

  // 各Moveが正しい型であることを確認
  for (const auto& move : moves) {
    const auto* event_move =
        dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
    EXPECT_NE(event_move, nullptr);
  }
}