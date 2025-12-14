// File: tests/game_state/basic_event_cards_test/mid_war.cpp
// Summary: Mid War期の基本イベントカードテストをまとめる。
// Reason: テスト肥大化を防ぎ、Mid Warカード検証を集約するため。

#include <gtest/gtest.h>

#include "basic_event_card_test_base.hpp"
#include "place_cards_test.hpp"
#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

using TheVoiceOfAmericaTest = BasicEventCardTestBase<TheVoiceOfAmerica>;

TEST_F(TheVoiceOfAmericaTest, EventReturnsRequestCommand) {
  auto commands = sut.event(Side::USA, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(TheVoiceOfAmericaTest, GeneratesRemoveMovesForNonEurope) {
  // 欧州以外に露影響力を設定
  board.getWorldMap().getCountry(CountryEnum::IRAN).addInfluence(Side::USSR, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::EGYPT)
      .addInfluence(Side::USSR, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::JAPAN)
      .addInfluence(Side::USSR, 2);

  // 欧州にも設定（除外されるべき）
  board.getWorldMap()
      .getCountry(CountryEnum::FRANCE)
      .addInfluence(Side::USSR, 2);

  auto commands = sut.event(Side::USA, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  ASSERT_NE(request_cmd, nullptr);

  auto legal_moves = request_cmd->legalMoves(board);
  EXPECT_FALSE(legal_moves.empty());

  // 全てEventRemoveInfluenceMoveであることを確認
  for (const auto& move : legal_moves) {
    auto* remove_move = dynamic_cast<EventRemoveInfluenceMove*>(move.get());
    EXPECT_NE(remove_move, nullptr);
  }
}

using MuslimRevolutionTest = BasicEventCardTestBase<MuslimRevolution>;

TEST_F(MuslimRevolutionTest, EventReturnsRequestCommand) {
  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(MuslimRevolutionTest, GeneratesRemoveAllMovesForTargetCountries) {
  // 対象8カ国のうち複数に米影響力を設定
  board.getWorldMap().getCountry(CountryEnum::IRAN).addInfluence(Side::USA, 2);
  board.getWorldMap().getCountry(CountryEnum::EGYPT).addInfluence(Side::USA, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::SAUDI_ARABIA)
      .addInfluence(Side::USA, 2);

  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  ASSERT_NE(request_cmd, nullptr);

  auto legal_moves = request_cmd->legalMoves(board);
  EXPECT_FALSE(legal_moves.empty());

  // 全てEventRemoveAllInfluenceMoveであることを確認
  for (const auto& move : legal_moves) {
    auto* remove_all_move =
        dynamic_cast<EventRemoveAllInfluenceMove*>(move.get());
    EXPECT_NE(remove_all_move, nullptr);
  }

  // 2カ国選択なので、3C2 = 3パターン
  EXPECT_EQ(legal_moves.size(), 3);
}

TEST_F(PlaceCardsTest, PuppetGovernmentsEventTest) {
  PuppetGovernments puppet_governments;

  EXPECT_TRUE(puppet_governments.canEvent(board));

  auto commands = puppet_governments.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, LiberationTheologyEventTest) {
  LiberationTheology liberation_theology;

  EXPECT_TRUE(liberation_theology.canEvent(board));

  auto commands = liberation_theology.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, OASFoundedEventTest) {
  OASFounded oas_founded;

  EXPECT_TRUE(oas_founded.canEvent(board));

  auto commands = oas_founded.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, ColonialRearGuardsEventTest) {
  ColonialRearGuards colonial_rear_guards;

  EXPECT_TRUE(colonial_rear_guards.canEvent(board));

  auto commands = colonial_rear_guards.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, UssuriRiverSkirmishEventTest) {
  UssuriRiverSkirmish ussuri_river_skirmish;

  EXPECT_TRUE(ussuri_river_skirmish.canEvent(board));

  auto commands = ussuri_river_skirmish.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, SouthAfricanUnrestEventTest) {
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
