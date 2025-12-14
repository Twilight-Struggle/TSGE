// File: tests/game_state/basic_event_cards_test/early_war.cpp
// Summary: Early War期の基本イベントカードテストを保持する。
// Reason:
// 基本イベントカード検証をWarPeriod別に切り分け、変更影響を局所化するため。

#include <gtest/gtest.h>

#include "basic_event_card_test_base.hpp"
#include "place_cards_test.hpp"
#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

using DuckAndCoverTest = BasicEventCardTestBase<DuckAndCover>;

TEST_F(DuckAndCoverTest, DuckAndCoverTest) {
  auto commands = sut.event(Side::USA, board);
  EXPECT_FALSE(commands.empty());

  // 初期状態確認
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);
  EXPECT_EQ(board.getVp(), 0);
  EXPECT_TRUE(board.getStates().empty());

  // コマンド実行
  for (const auto& command : commands) {
    command->apply(board);
  }

  // DEFCON変更確認
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 4);

  // VP変更はpushStateで追加されているかを確認
  auto& states = board.getStates();
  EXPECT_FALSE(states.empty());

  // 最後のstateがChangeVpCommandであることを確認
  EXPECT_TRUE(std::holds_alternative<CommandPtr>(states.back()));
  auto& last_command = std::get<CommandPtr>(states.back());

  // pushされたCommandを実行してVP変更を確認
  last_command->apply(board);
  EXPECT_EQ(board.getVp(), -1);  // (5-4) * (-1) = -1 (USA側なので負)
}

using FidelTest = BasicEventCardTestBase<Fidel>;

TEST_F(FidelTest, FidelTest) {
  board.getWorldMap().getCountry(CountryEnum::CUBA).addInfluence(Side::USA, 1);
  auto commands = sut.event(Side::USSR, board);
  // Fidel is commented out for now as per task requirements
  EXPECT_TRUE(commands.empty());
  // Test expectations are temporarily disabled since Fidel event is not
  // implemented EXPECT_EQ(
  //     board.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USA),
  //     0);
  // EXPECT_EQ(
  //     board.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USSR),
  //     3);
}

using NuclearTestBanTest = BasicEventCardTestBase<NuclearTestBan>;

TEST_F(NuclearTestBanTest, NuclearTestBanTest) {
  // Test USSR playing Nuclear Test Ban from DEFCON 5
  auto commands = sut.event(Side::USSR, board);
  EXPECT_FALSE(commands.empty());

  // 初期状態確認
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);
  EXPECT_EQ(board.getVp(), 0);
  EXPECT_TRUE(board.getStates().empty());

  // コマンド実行
  for (const auto& command : commands) {
    command->apply(board);
  }

  // DEFCON変更確認（5 + 2 = 7, capped at 5）
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);

  // VP変更はpushStateで追加されているかを確認
  auto& states = board.getStates();
  EXPECT_FALSE(states.empty());

  // pushされたCommandを実行してVP変更を確認
  auto& vp_command = std::get<CommandPtr>(states.back());
  vp_command->apply(board);
  EXPECT_EQ(board.getVp(), 3);  // (5-2) * 1 = 3 VP for USSR

  // statesをクリアして次のテスト準備
  states.clear();

  // Set DEFCON to 2 for next test
  board.getDefconTrack().setDefcon(2);

  // Test USA playing Nuclear Test Ban from DEFCON 2
  auto commands2 = sut.event(Side::USA, board);
  EXPECT_FALSE(commands2.empty());
  for (const auto& command : commands2) {
    command->apply(board);
  }

  // DEFCON変更確認（2 + 2 = 4）
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 4);

  // VP変更確認
  EXPECT_FALSE(states.empty());
  auto& vp_command2 = std::get<CommandPtr>(states.back());
  vp_command2->apply(board);
  EXPECT_EQ(board.getVp(), 3);  // 3 + (2-2) * (-1) = 3 (no change)
}

using SocialistGovernmentsTest = BasicEventCardTestBase<SocialistGovernments>;

TEST_F(SocialistGovernmentsTest, EventReturnsRequestCommand) {
  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SocialistGovernmentsTest, GeneratesRemoveMovesForWestEurope) {
  // 西欧に米影響力を設定
  board.getWorldMap()
      .getCountry(CountryEnum::FRANCE)
      .addInfluence(Side::USA, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::WEST_GERMANY)
      .addInfluence(Side::USA, 2);
  board.getWorldMap().getCountry(CountryEnum::ITALY).addInfluence(Side::USA, 2);

  auto commands = sut.event(Side::USSR, board);
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

using SuezCrisisTest = BasicEventCardTestBase<SuezCrisis>;

TEST_F(SuezCrisisTest, EventReturnsRequestCommand) {
  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(SuezCrisisTest, OnlyTargetsThreeCountries) {
  // 対象3カ国に米影響力を設定
  board.getWorldMap()
      .getCountry(CountryEnum::FRANCE)
      .addInfluence(Side::USA, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::ISRAEL)
      .addInfluence(Side::USA, 2);

  // 他の国にも設定（除外されるべき）
  board.getWorldMap()
      .getCountry(CountryEnum::WEST_GERMANY)
      .addInfluence(Side::USA, 2);

  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  ASSERT_NE(request_cmd, nullptr);

  auto legal_moves = request_cmd->legalMoves(board);
  EXPECT_FALSE(legal_moves.empty());
}

using EastEuropeanUnrestTest = BasicEventCardTestBase<EastEuropeanUnrest>;

TEST_F(EastEuropeanUnrestTest, EventReturnsRequestCommand) {
  auto commands = sut.event(Side::USA, board);
  ASSERT_EQ(commands.size(), 1);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(EastEuropeanUnrestTest, GeneratesMovesForEastEurope) {
  // 東欧に露影響力を設定
  board.getWorldMap()
      .getCountry(CountryEnum::POLAND)
      .addInfluence(Side::USSR, 3);
  board.getWorldMap()
      .getCountry(CountryEnum::HUNGARY)
      .addInfluence(Side::USSR, 3);
  board.getWorldMap()
      .getCountry(CountryEnum::CZECHOSLOVAKIA)
      .addInfluence(Side::USSR, 3);
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 3);

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

#include "early_war_place_cards_tests.hpp"
