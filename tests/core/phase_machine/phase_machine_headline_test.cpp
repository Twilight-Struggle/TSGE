#include "phase_machine_test_helper.hpp"

// ヘッドラインフェイズの基本テスト
TEST_F(PhaseMachineTest, HeadlinePhaseBasicFlow) {
  // プレイヤーに手札を追加
  auto execution_log =
      std::make_shared<std::vector<std::pair<Side, CardEnum>>>();
  auto& pool = defaultCardPool();
  CardPoolGuard ussr_guard(pool, CardEnum::DUCK_AND_COVER,
                           std::make_unique<HeadlineEventCard>(
                               CardEnum::DUCK_AND_COVER, "USSR Headline", 2,
                               Side::USSR, execution_log));
  CardPoolGuard usa_guard(
      pool, CardEnum::FIDEL,
      std::make_unique<HeadlineEventCard>(CardEnum::FIDEL, "USA Headline", 4,
                                          Side::USA, execution_log));

  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USSR, CardEnum::NUCLEAR_TEST_BAN);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);
  board.addCardToHand(Side::USA, CardEnum::NUCLEAR_TEST_BAN);

  prepareDeckWithDummyCards(40);
  const int current_turn = board.getTurnTrack().getTurn();
  const auto draw_counts = board.calculateDrawCount(current_turn);
  const int expected_ussr_after_draw =
      static_cast<int>(board.getPlayerHand(Side::USSR).size()) + draw_counts[0];
  const int expected_usa_after_draw =
      static_cast<int>(board.getPlayerHand(Side::USA).size()) + draw_counts[1];

  // TURN_STARTをプッシュしてヘッドラインフェイズをトリガー
  board.pushState(StateType::TURN_START);

  // TURN_STARTを実行してHEADLINE_PHASEがプッシュされることを確認
  auto first_result = PhaseMachine::step(board, std::nullopt);

  auto& ussr_moves = std::get<0>(first_result);
  ASSERT_EQ(ussr_moves.size(), expected_ussr_after_draw);
  auto ussr_move_it =
      std::find_if(ussr_moves.begin(), ussr_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::DUCK_AND_COVER;
      });
  ASSERT_NE(ussr_move_it, ussr_moves.end());
  auto ussr_move = *ussr_move_it;

  // HEADLINE_CARD_SELECT_USSRが返されることを期待（同時選択の疑似実装）
  EXPECT_EQ(std::get<1>(first_result), Side::USSR);
  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), expected_ussr_after_draw);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw);
  EXPECT_TRUE(ussr_move);

  auto second_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(ussr_move)));

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(),
            expected_ussr_after_draw - 1);
  EXPECT_EQ(board.getHeadlineCard(Side::USSR), CardEnum::DUCK_AND_COVER);

  auto& usa_moves = std::get<0>(second_result);
  ASSERT_EQ(usa_moves.size(), expected_usa_after_draw);
  auto usa_move_it = std::find_if(
      usa_moves.begin(), usa_moves.end(),
      [](const auto& move) { return move->getCard() == CardEnum::FIDEL; });
  ASSERT_NE(usa_move_it, usa_moves.end());
  auto usa_move = *usa_move_it;
  EXPECT_EQ(std::get<1>(second_result), Side::USA);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw);
  EXPECT_TRUE(usa_move);

  auto third_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(usa_move)));

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(),
            expected_ussr_after_draw - 1);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw - 1);
  EXPECT_EQ(std::get<1>(third_result), Side::USSR);
  const auto& ar_moves = std::get<0>(third_result);
  ASSERT_FALSE(ar_moves.empty());
  EXPECT_TRUE(
      std::any_of(ar_moves.begin(), ar_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::NUCLEAR_TEST_BAN &&
               move->getSide() == Side::USSR;
      }));
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
  EXPECT_EQ(board.getHeadlineCard(Side::USSR), CardEnum::DUMMY);
  EXPECT_EQ(board.getHeadlineCard(Side::USA), CardEnum::DUMMY);

  const std::vector<std::pair<Side, CardEnum>> expected_log = {
      {Side::USA, CardEnum::FIDEL}, {Side::USSR, CardEnum::DUCK_AND_COVER}};
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
  CardPoolGuard ussr_guard(pool, CardEnum::DUCK_AND_COVER,
                           std::make_unique<HeadlineEventCard>(
                               CardEnum::DUCK_AND_COVER, "USSR Headline", 2,
                               Side::USSR, execution_log, false));
  CardPoolGuard usa_guard(
      pool, CardEnum::FIDEL,
      std::make_unique<HeadlineEventCard>(CardEnum::FIDEL, "USA Headline", 4,
                                          Side::USA, execution_log, true));

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USSR, CardEnum::NUCLEAR_TEST_BAN);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);
  board.addCardToHand(Side::USA, CardEnum::NUCLEAR_TEST_BAN);

  prepareDeckWithDummyCards(40);
  const int current_turn = board.getTurnTrack().getTurn();
  const auto draw_counts = board.calculateDrawCount(current_turn);
  const int expected_ussr_after_draw =
      static_cast<int>(board.getPlayerHand(Side::USSR).size()) + draw_counts[0];
  const int expected_usa_after_draw =
      static_cast<int>(board.getPlayerHand(Side::USA).size()) + draw_counts[1];

  board.pushState(StateType::TURN_START);

  auto first_result = PhaseMachine::step(board, std::nullopt);
  auto& ussr_moves = std::get<0>(first_result);
  ASSERT_EQ(ussr_moves.size(), expected_ussr_after_draw);
  auto ussr_move_it =
      std::find_if(ussr_moves.begin(), ussr_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::DUCK_AND_COVER;
      });
  ASSERT_NE(ussr_move_it, ussr_moves.end());
  auto ussr_move = *ussr_move_it;

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), expected_ussr_after_draw);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw);

  auto second_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(ussr_move)));
  auto& usa_moves = std::get<0>(second_result);
  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(),
            expected_ussr_after_draw - 1);
  ASSERT_EQ(usa_moves.size(), expected_usa_after_draw);
  auto usa_move_it = std::find_if(
      usa_moves.begin(), usa_moves.end(),
      [](const auto& move) { return move->getCard() == CardEnum::FIDEL; });
  ASSERT_NE(usa_move_it, usa_moves.end());
  auto usa_move = *usa_move_it;

  EXPECT_EQ(board.getHeadlineCard(Side::USSR), CardEnum::DUCK_AND_COVER);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw);

  auto third_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(usa_move)));

  // ヘッドライン処理後も各プレイヤーに1枚ずつ手札が残り、ARへ移行できる
  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(),
            expected_ussr_after_draw - 1);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw - 1);

  const auto& discard = board.getDeck().getDiscardPile();
  ASSERT_EQ(discard.size(), 1);
  EXPECT_EQ(discard.front(), CardEnum::DUCK_AND_COVER);

  const auto& removed = board.getDeck().getRemovedCards();
  ASSERT_EQ(removed.size(), 1);
  EXPECT_EQ(removed.front(), CardEnum::FIDEL);

  // AR入力を要求する状態へ遷移し、USSRが行動待ちになる
  EXPECT_EQ(std::get<1>(third_result), Side::USSR);
  const auto& ar_moves_for_guard = std::get<0>(third_result);
  ASSERT_FALSE(ar_moves_for_guard.empty());
  EXPECT_TRUE(std::any_of(ar_moves_for_guard.begin(), ar_moves_for_guard.end(),
                          [](const auto& move) {
                            return move->getCard() ==
                                       CardEnum::NUCLEAR_TEST_BAN &&
                                   move->getSide() == Side::USSR;
                          }));
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
}

// 発動不可カードでも解決後に正しく捨て札へ移動することを確認する
TEST_F(PhaseMachineTest, HeadlineCardsWithoutEventGoToDiscard) {
  auto execution_log =
      std::make_shared<std::vector<std::pair<Side, CardEnum>>>();
  auto& pool = defaultCardPool();
  CardPoolGuard ussr_guard(pool, CardEnum::DUCK_AND_COVER,
                           std::make_unique<HeadlineEventCard>(
                               CardEnum::DUCK_AND_COVER, "USSR Headline", 2,
                               Side::USSR, execution_log, true, false));
  CardPoolGuard usa_guard(pool, CardEnum::FIDEL,
                          std::make_unique<HeadlineEventCard>(
                              CardEnum::FIDEL, "USA Headline", 4, Side::USA,
                              execution_log, false, true));

  board.clearHand(Side::USSR);
  board.clearHand(Side::USA);
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USSR, CardEnum::NUCLEAR_TEST_BAN);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);
  board.addCardToHand(Side::USA, CardEnum::NUCLEAR_TEST_BAN);

  prepareDeckWithDummyCards(40);
  const int current_turn = board.getTurnTrack().getTurn();
  const auto draw_counts = board.calculateDrawCount(current_turn);
  const int expected_ussr_after_draw =
      static_cast<int>(board.getPlayerHand(Side::USSR).size()) + draw_counts[0];
  const int expected_usa_after_draw =
      static_cast<int>(board.getPlayerHand(Side::USA).size()) + draw_counts[1];

  board.pushState(StateType::TURN_START);

  auto first_result = PhaseMachine::step(board, std::nullopt);
  auto& ussr_moves = std::get<0>(first_result);
  ASSERT_EQ(ussr_moves.size(), expected_ussr_after_draw);
  auto ussr_move_it =
      std::find_if(ussr_moves.begin(), ussr_moves.end(), [](const auto& move) {
        return move->getCard() == CardEnum::DUCK_AND_COVER;
      });
  ASSERT_NE(ussr_move_it, ussr_moves.end());
  auto ussr_move = *ussr_move_it;

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(), expected_ussr_after_draw);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw);

  auto second_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(ussr_move)));
  auto& usa_moves = std::get<0>(second_result);
  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(),
            expected_ussr_after_draw - 1);
  ASSERT_EQ(usa_moves.size(), expected_usa_after_draw);
  auto usa_move_it = std::find_if(
      usa_moves.begin(), usa_moves.end(),
      [](const auto& move) { return move->getCard() == CardEnum::FIDEL; });
  ASSERT_NE(usa_move_it, usa_moves.end());
  auto usa_move = *usa_move_it;

  auto third_result = PhaseMachine::step(
      board, std::optional<std::shared_ptr<Move>>(std::move(usa_move)));

  EXPECT_EQ(board.getPlayerHand(Side::USSR).size(),
            expected_ussr_after_draw - 1);
  EXPECT_EQ(board.getPlayerHand(Side::USA).size(), expected_usa_after_draw - 1);

  const auto& discard = board.getDeck().getDiscardPile();
  ASSERT_EQ(discard.size(), 2);
  EXPECT_EQ(
      std::count(discard.begin(), discard.end(), CardEnum::DUCK_AND_COVER), 1);
  EXPECT_EQ(std::count(discard.begin(), discard.end(), CardEnum::FIDEL), 1);

  const auto& removed = board.getDeck().getRemovedCards();
  EXPECT_TRUE(removed.empty());

  // USSRカードは発動していないためログにはUSAカードのみ記録される
  ASSERT_EQ(execution_log->size(), 1);
  EXPECT_EQ(execution_log->front(), std::make_pair(Side::USA, CardEnum::FIDEL));

  EXPECT_EQ(std::get<1>(third_result), Side::USSR);
  const auto& ar_moves_without_event = std::get<0>(third_result);
  ASSERT_FALSE(ar_moves_without_event.empty());
  EXPECT_TRUE(std::any_of(ar_moves_without_event.begin(),
                          ar_moves_without_event.end(), [](const auto& move) {
                            return move->getCard() ==
                                       CardEnum::NUCLEAR_TEST_BAN &&
                                   move->getSide() == Side::USSR;
                          }));
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);
}

// 宇宙開発トラック優位性があるケースのテスト
TEST_F(PhaseMachineTest, HeadlinePhaseSpaceAdvantage) {
  // プレイヤーに手札を追加
  board.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board.addCardToHand(Side::USA, CardEnum::FIDEL);

  // USSRが宇宙開発トラック4に到達
  board.getSpaceTrack().advanceSpaceTrack(Side::USSR, 4);

  // HEADLINE_PHASEを直接実行
  board.pushState(StateType::HEADLINE_PHASE);

  auto result = PhaseMachine::step(board, std::nullopt);

  // USAが先に選択することを期待（劣位側が先）
  EXPECT_EQ(std::get<1>(result), Side::USA);
  EXPECT_FALSE(std::get<0>(result).empty());
}
