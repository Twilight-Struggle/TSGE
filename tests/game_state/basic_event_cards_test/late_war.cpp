// File: tests/game_state/basic_event_cards_test/late_war.cpp
// Summary: Late War期の基本イベントカードテストを収容する。
// Reason: Late Warカード挙動を独立に検証し保守性を向上させるため。

#include <gtest/gtest.h>

#include "basic_event_card_test_base.hpp"
#include "place_cards_test.hpp"
#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

using PershingIIDeployedTest = BasicEventCardTestBase<PershingIIDeployed>;

TEST_F(PershingIIDeployedTest, PershingIIDeployedVpTest) {
  // Pershing II Deployed: USSRが1VP獲得 + 西欧3カ国から米影響力各1除去
  auto commands = sut.event(Side::USSR, board);
  EXPECT_FALSE(commands.empty());
  // 2つのコマンド: ChangeVpCommand + RequestCommand
  EXPECT_EQ(commands.size(), 2);

  // 初期状態確認
  EXPECT_EQ(board.getVp(), 0);

  // 最初のコマンド（ChangeVpCommand）を実行
  commands[0]->apply(board);

  // USSRが1VP獲得（VP+1）
  EXPECT_EQ(board.getVp(), 1);
}

TEST_F(PershingIIDeployedTest, PershingIIDeployedRequestTest) {
  // 西欧に米影響力を設定
  board.getWorldMap()
      .getCountry(CountryEnum::FRANCE)
      .addInfluence(Side::USA, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::WEST_GERMANY)
      .addInfluence(Side::USA, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 2);
  board.getWorldMap().getCountry(CountryEnum::ITALY).addInfluence(Side::USA, 2);

  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 2);

  // 2番目のコマンドがRequestCommand
  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  // RequestCommandから合法手を取得
  auto legal_moves = request_cmd->legalMoves(board);
  EXPECT_FALSE(legal_moves.empty());

  // 合法手がEventRemoveInfluenceMoveであることを確認
  for (const auto& move : legal_moves) {
    auto* remove_move = dynamic_cast<EventRemoveInfluenceMove*>(move.get());
    EXPECT_NE(remove_move, nullptr);
  }
}

using MarineBarracksBombingTest = BasicEventCardTestBase<MarineBarracksBombing>;

TEST_F(MarineBarracksBombingTest, EventReturnsTwoCommands) {
  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 2);

  // 最初のコマンドがRemoveAllInfluenceCommand
  auto* remove_all_cmd =
      dynamic_cast<RemoveAllInfluenceCommand*>(commands[0].get());
  EXPECT_NE(remove_all_cmd, nullptr);

  // 2番目のコマンドがRequestCommand
  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(MarineBarracksBombingTest, RemovesAllUSAInfluenceFromLebanon) {
  // レバノンに米影響力を設定
  board.getWorldMap()
      .getCountry(CountryEnum::LEBANON)
      .addInfluence(Side::USA, 3);

  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 2);

  // 最初のコマンドを実行
  commands[0]->apply(board);

  // レバノンの米影響力が0になっていることを確認
  EXPECT_EQ(board.getWorldMap()
                .getCountry(CountryEnum::LEBANON)
                .getInfluence(Side::USA),
            0);
}

TEST_F(MarineBarracksBombingTest, RequestCommandGeneratesMiddleEastMoves) {
  // 中東に米影響力を設定
  board.getWorldMap().getCountry(CountryEnum::IRAN).addInfluence(Side::USA, 2);
  board.getWorldMap().getCountry(CountryEnum::IRAQ).addInfluence(Side::USA, 2);

  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 2);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  auto legal_moves = request_cmd->legalMoves(board);
  EXPECT_FALSE(legal_moves.empty());
}

TEST_F(PlaceCardsTest, TheReformerEventTest) {
  TheReformer the_reformer;

  EXPECT_TRUE(the_reformer.canEvent(board));

  auto commands = the_reformer.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}
