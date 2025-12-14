#pragma once
// File: tests/game_state/basic_event_cards_test/early_war_place_cards_tests.hpp
// Summary: Early War期に属するPlace系カードテストを切り出し共有する。
// Reason:
// early_war.cppの行数を抑制しつつ、PlaceCardsTestを再利用しやすくするため。

TEST_F(PlaceCardsTest, ComeconEventTest) {
  Comecon comecon;

  EXPECT_TRUE(comecon.canEvent(board));

  auto commands = comecon.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, DecolonizationEventTest) {
  Decolonization decolonization;

  EXPECT_TRUE(decolonization.canEvent(board));

  auto commands = decolonization.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, MarshallPlanEventTest) {
  MarshallPlan marshall_plan;

  EXPECT_TRUE(marshall_plan.canEvent(board));

  auto commands = marshall_plan.event(Side::USA, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, WarsawPactFormedEventTest) {
  WarsawPactFormed warsaw_pact;

  EXPECT_TRUE(warsaw_pact.canEvent(board));

  auto commands = warsaw_pact.event(Side::USSR, board);
  EXPECT_EQ(commands.size(), 1);

  const auto* request_cmd =
      dynamic_cast<const RequestCommand*>(commands[0].get());
  EXPECT_NE(request_cmd, nullptr);
}

TEST_F(PlaceCardsTest, SpecialRelationshipEventTest) {
  SpecialRelationship special_relationship;

  EXPECT_TRUE(special_relationship.canEvent(board));

  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 5);

  {
    auto commands = special_relationship.event(Side::USA, board);
    EXPECT_EQ(commands.size(), 1);

    const auto* request_cmd =
        dynamic_cast<const RequestCommand*>(commands[0].get());
    ASSERT_NE(request_cmd, nullptr);

    auto moves = request_cmd->legalMoves(board);
    EXPECT_GT(moves.size(), 0);

    for (const auto& move : moves) {
      const auto* event_move =
          dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
      EXPECT_NE(event_move, nullptr);
    }
  }

  {
    board.addCardEffectInProgress(CardEnum::NATO);
    auto commands = special_relationship.event(Side::USA, board);
    EXPECT_EQ(commands.size(), 2);

    const auto* request_cmd =
        dynamic_cast<const RequestCommand*>(commands[0].get());
    ASSERT_NE(request_cmd, nullptr);

    auto moves = request_cmd->legalMoves(board);
    EXPECT_GT(moves.size(), 0);

    for (const auto& move : moves) {
      const auto* event_move =
          dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
      EXPECT_NE(event_move, nullptr);
    }

    const auto* vp_cmd =
        dynamic_cast<const ChangeVpCommand*>(commands[1].get());
    ASSERT_NE(vp_cmd, nullptr);
  }
}
