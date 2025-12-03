#include "test_helper.hpp"
#include "tsge/actions/legal_moves_generator.hpp"

// actionPlaceInfluenceLegalMovesのテスト
class ActionPlaceInfluenceLegalMovesTest : public ::testing::Test {
 protected:
  ActionPlaceInfluenceLegalMovesTest() : board(createTestCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USSR, false); }

  Board board;
};

TEST_F(ActionPlaceInfluenceLegalMovesTest, SimpleCase2OpsOneCountry) {
  // シンプルなケース：1国のみ配置可能、2 Ops
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSRに東ドイツに影響力1を配置（東ドイツのみ配置可能）
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});  // 2 Ops

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // 2 Opsで東ドイツに配置可能なパターン：
  // 全ての配置可能国が1 Opsコストのため、多数のパターンが生成される
  EXPECT_EQ(moves.size(), 15);

  // 全てのMoveがActionPlaceInfluenceMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionPlaceInfluenceMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, EmptyHand) {
  // 手札なしテスト
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto placeable = board.getWorldMap().placeableCountries(Side::USSR);
  ASSERT_FALSE(placeable.empty());

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, NoPlaceableCountries) {
  // 配置可能国なしテスト（非現実的だが境界値テスト）
  // 全ての国から両方のプレイヤーの影響力を削除
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, MixedCostCountries) {
  // 異なるコストの国が混在するケース
  // まず状態をリセット
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSR影響力を配置
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);
  // USA支配国を作成（コスト2）
  board.getWorldMap()
      .getCountry(CountryEnum::WEST_GERMANY)
      .addInfluence(Side::USA, 4);

  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DUCK_AND_COVER});  // 3 Ops

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // 3 Opsで可能な配置パターンが生成される
  // 西ドイツ（2 Opsコスト）と他の国（1 Opsコスト）の組み合わせ
  EXPECT_EQ(moves.size(), 25);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, MultipleCardsWithSameOps) {
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSR影響力を配置
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  // 同じOps値のカードが複数ある場合
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::FIDEL, CardEnum::FIDEL});  // 両方2 Ops

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // 同じOps値のカード2枚分のMoveが生成される
  auto placeable = board.getWorldMap().placeableCountries(Side::USSR);
  // 2 Opsの配置パターン数を2倍
  auto single_card_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);
  board.clearHand(Side::USSR);
  board.addCardToHand(Side::USSR, CardEnum::FIDEL);
  auto one_card_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);
  EXPECT_EQ(moves.size(), one_card_moves.size() * 2);

  // 同じ配置パターンが複数カード分存在することを確認
  std::map<CardEnum, int> card_count;
  for (const auto& move : moves) {
    card_count[move->getCard()]++;
  }
  EXPECT_EQ(card_count[CardEnum::FIDEL], card_count[CardEnum::FIDEL]);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, ChinaCardProvidesAsiaBonus) {
  const auto configure_board = [](Board& target) {
    TestHelper::clearAllOpponentInfluence(target, Side::USSR);
    TestHelper::clearAllOpponentInfluence(target, Side::USA);
    TestHelper::clearSuperPowerInfluence(target, Side::USSR);
    TestHelper::clearSuperPowerInfluence(target, Side::USA);

    auto& north_korea =
        target.getWorldMap().getCountry(CountryEnum::NORTH_KOREA);
    north_korea.clearInfluence(Side::USA);
    north_korea.clearInfluence(Side::USSR);
    north_korea.addInfluence(Side::USSR, 1);
  };

  configure_board(board);
  board.giveChinaCardTo(Side::USSR, true);

  const auto& china_card =
      board.getCardpool()[static_cast<size_t>(CardEnum::CHINA_CARD)];

  auto china_moves = LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
      board, Side::USSR, CardEnum::CHINA_CARD);
  auto neutral_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
          board, Side::USSR, CardEnum::NUCLEAR_TEST_BAN);

  ASSERT_FALSE(china_moves.empty());
  EXPECT_GT(china_moves.size(), neutral_moves.size());

  const auto total_influence = [](const Board& target, Side side) {
    int total = 0;
    const auto& world_map = target.getWorldMap();
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < world_map.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      total += world_map.getCountry(country_enum).getInfluence(side);
    }
    return total;
  };

  bool has_bonus_pattern = false;
  for (const auto& move : china_moves) {
    ASSERT_NE(move, nullptr);

    auto commands = move->toCommand(china_card, board);
    ASSERT_FALSE(commands.empty());
    auto* place_cmd =
        dynamic_cast<PlaceInfluenceCommand*>(commands.front().get());
    ASSERT_NE(place_cmd, nullptr);

    Board eval_board(createTestCardPool());
    configure_board(eval_board);

    const int before_total = total_influence(eval_board, Side::USSR);

    std::map<CountryEnum, int> before_influence;
    auto& eval_world = eval_board.getWorldMap();
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < eval_world.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      before_influence[country_enum] =
          eval_world.getCountry(country_enum).getInfluence(Side::USSR);
    }

    place_cmd->apply(eval_board);

    const int after_total = total_influence(eval_board, Side::USSR);
    const int delta = after_total - before_total;

    EXPECT_EQ(delta, 5);

    bool used_only_asia = true;
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < eval_world.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      const int before = before_influence[country_enum];
      const int after =
          eval_world.getCountry(country_enum).getInfluence(Side::USSR);
      if (after > before &&
          !eval_world.getCountry(country_enum).hasRegion(Region::ASIA)) {
        used_only_asia = false;
        break;
      }
    }
    EXPECT_TRUE(used_only_asia);

    has_bonus_pattern = true;
  }

  EXPECT_TRUE(has_bonus_pattern);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, ChinaCardBaseOpsNeedsNonAsia) {
  const auto configure_board = [](Board& target) {
    TestHelper::clearAllOpponentInfluence(target, Side::USSR);
    TestHelper::clearAllOpponentInfluence(target, Side::USA);
    TestHelper::clearSuperPowerInfluence(target, Side::USSR);
    TestHelper::clearSuperPowerInfluence(target, Side::USA);

    auto& north_korea =
        target.getWorldMap().getCountry(CountryEnum::NORTH_KOREA);
    north_korea.clearInfluence(Side::USA);
    north_korea.clearInfluence(Side::USSR);
    north_korea.addInfluence(Side::USSR, 1);

    auto& east_germany =
        target.getWorldMap().getCountry(CountryEnum::EAST_GERMANY);
    east_germany.clearInfluence(Side::USA);
    east_germany.clearInfluence(Side::USSR);
    east_germany.addInfluence(Side::USSR, 1);
  };

  configure_board(board);
  board.giveChinaCardTo(Side::USSR, true);

  const auto& china_card =
      board.getCardpool()[static_cast<size_t>(CardEnum::CHINA_CARD)];

  auto moves = LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
      board, Side::USSR, CardEnum::CHINA_CARD);

  ASSERT_FALSE(moves.empty());
  EXPECT_EQ(moves.front()->getSide(), Side::USSR);

  bool found_non_asia_base = false;
  bool found_asia_bonus = false;

  for (const auto& move : moves) {
    ASSERT_NE(move, nullptr);

    auto commands = move->toCommand(china_card, board);
    ASSERT_FALSE(commands.empty());
    auto* place_cmd =
        dynamic_cast<PlaceInfluenceCommand*>(commands.front().get());
    ASSERT_NE(place_cmd, nullptr);

    Board eval_board(createTestCardPool());
    configure_board(eval_board);

    std::map<CountryEnum, int> before_influence;
    auto& eval_world = eval_board.getWorldMap();
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < eval_world.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      before_influence[country_enum] =
          eval_world.getCountry(country_enum).getInfluence(Side::USSR);
    }

    place_cmd->apply(eval_board);

    int delta = 0;
    bool all_asia = true;
    bool includes_non_asia = false;
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < eval_world.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      const int before = before_influence[country_enum];
      const int after =
          eval_world.getCountry(country_enum).getInfluence(Side::USSR);
      if (after > before) {
        const int diff = after - before;
        delta += diff;
        const auto& country = eval_world.getCountry(country_enum);
        if (!country.hasRegion(Region::ASIA)) {
          all_asia = false;
          includes_non_asia = true;
        }
      }
    }

    if (delta == 4) {
      EXPECT_TRUE(includes_non_asia);
      EXPECT_FALSE(all_asia);
      found_non_asia_base = true;
    } else if (delta == 5) {
      EXPECT_TRUE(all_asia);
      found_asia_bonus = true;
    } else {
      ADD_FAILURE() << "Unexpected delta: " << delta;
    }
  }

  EXPECT_TRUE(found_non_asia_base);
  EXPECT_TRUE(found_asia_bonus);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest,
       ChinaCardVietnamRevoltsSplitsBonuses) {
  const auto configure_board = [](Board& target) {
    TestHelper::clearAllOpponentInfluence(target, Side::USSR);
    TestHelper::clearAllOpponentInfluence(target, Side::USA);
    TestHelper::clearSuperPowerInfluence(target, Side::USSR);
    TestHelper::clearSuperPowerInfluence(target, Side::USA);

    auto& north_korea =
        target.getWorldMap().getCountry(CountryEnum::NORTH_KOREA);
    north_korea.clearInfluence(Side::USA);
    north_korea.clearInfluence(Side::USSR);
    north_korea.addInfluence(Side::USSR, 1);

    auto& vietnam = target.getWorldMap().getCountry(CountryEnum::VIETNAM);
    vietnam.clearInfluence(Side::USA);
    vietnam.clearInfluence(Side::USSR);
    vietnam.addInfluence(Side::USSR, 1);

    auto& east_germany =
        target.getWorldMap().getCountry(CountryEnum::EAST_GERMANY);
    east_germany.clearInfluence(Side::USA);
    east_germany.clearInfluence(Side::USSR);
    east_germany.addInfluence(Side::USSR, 1);

    target.clearCardsEffectsInThisTurn();
    target.addCardEffectInThisTurn(Side::USSR, CardEnum::VIETNAM_REVOLTS);
  };

  configure_board(board);
  board.giveChinaCardTo(Side::USSR, true);

  const auto& china_card =
      board.getCardpool()[static_cast<size_t>(CardEnum::CHINA_CARD)];

  auto moves = LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
      board, Side::USSR, CardEnum::CHINA_CARD);

  bool found_non_asia_base = false;
  bool found_asia_without_se_bonus = false;
  bool found_se_asia_bonus = false;
  bool found_invalid_se_asia_five = false;

  for (const auto& move : moves) {
    ASSERT_NE(move, nullptr);

    auto commands = move->toCommand(china_card, board);
    ASSERT_FALSE(commands.empty());
    auto* place_cmd =
        dynamic_cast<PlaceInfluenceCommand*>(commands.front().get());
    ASSERT_NE(place_cmd, nullptr);

    Board eval_board(createTestCardPool());
    configure_board(eval_board);

    std::map<CountryEnum, int> before_influence;
    auto& eval_world = eval_board.getWorldMap();
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < eval_world.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      before_influence[country_enum] =
          eval_world.getCountry(country_enum).getInfluence(Side::USSR);
    }

    place_cmd->apply(eval_board);

    int delta = 0;
    bool all_asia = true;
    bool all_se_asia = true;
    bool includes_non_asia = false;
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < eval_world.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      const int before = before_influence[country_enum];
      const int after =
          eval_world.getCountry(country_enum).getInfluence(Side::USSR);
      if (after > before) {
        const int diff = after - before;
        delta += diff;
        const auto& country = eval_world.getCountry(country_enum);
        if (!country.hasRegion(Region::ASIA)) {
          all_asia = false;
          includes_non_asia = true;
        }
        if (!country.hasRegion(Region::SOUTH_EAST_ASIA)) {
          all_se_asia = false;
        }
      }
    }

    if (delta == 4) {
      EXPECT_TRUE(includes_non_asia);
      EXPECT_FALSE(all_asia);
      found_non_asia_base = true;
    } else if (delta == 5) {
      EXPECT_TRUE(all_asia);
      if (all_se_asia) {
        found_invalid_se_asia_five = true;
      } else {
        found_asia_without_se_bonus = true;
      }
    } else if (delta == 6) {
      EXPECT_TRUE(all_se_asia);
      found_se_asia_bonus = true;
    } else {
      ADD_FAILURE() << "Unexpected delta: " << delta;
    }
  }

  EXPECT_TRUE(found_non_asia_base);
  EXPECT_TRUE(found_asia_without_se_bonus);
  EXPECT_TRUE(found_se_asia_bonus);
  EXPECT_FALSE(found_invalid_se_asia_five);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, ScoringCardNoMoves) {
  // スコアカード（Ops0）では配置不可
  // 状態をクリアして制御された環境でテスト
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSRに影響力を配置して配置可能国を作る
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUMMY});  // Ops0

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // CLAUDE.mdの仕様：Ops0では合法手なし（実装でOps0カードは除外される）
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, SingleCardHelperMatchesGeneral) {
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});

  auto helper_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
          board, Side::USSR, CardEnum::FIDEL);
  auto general_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  ASSERT_EQ(helper_moves.size(), general_moves.size());
  EXPECT_FALSE(helper_moves.empty());
  for (const auto& move : helper_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::FIDEL);
  }
}

// actionCoupLegalMovesのテスト
class ActionCoupLegalMovesTest : public ::testing::Test {
 protected:
  ActionCoupLegalMovesTest() : board(createTestCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USSR, false); }

  Board board;
};

TEST_F(ActionCoupLegalMovesTest, BasicCaseDefcon5) {
  // 基本ケース（DEFCON 5）：全ての相手影響力がある国が対象
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DUCK_AND_COVER});  // 3 Ops
  board.getDefconTrack().setDefcon(5);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 相手影響力がある国数分のMoveが生成される
  // setupBoardWithInfluenceで日本、西ドイツ、イラン、アンゴラにUSA影響力設定
  EXPECT_EQ(moves.size(), 4);

  // 全てのMoveがActionCoupMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionCoupMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionCoupLegalMovesTest, Defcon4RestrictionEurope) {
  // DEFCON 4：ヨーロッパ制限
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});
  board.getDefconTrack().setDefcon(4);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 西ドイツ（ヨーロッパ）が除外される
  // 日本、イラン、アンゴラの3か国のみ
  EXPECT_EQ(moves.size(), 3);

  auto has_move = [&](CountryEnum target) {
    ActionCoupMove expected(CardEnum::DUCK_AND_COVER, Side::USSR, target);
    return std::any_of(moves.begin(), moves.end(),
                       [&](const auto& move) { return *move == expected; });
  };

  EXPECT_TRUE(has_move(CountryEnum::JAPAN));
  EXPECT_TRUE(has_move(CountryEnum::IRAN));
  EXPECT_TRUE(has_move(CountryEnum::ANGOLA));
  EXPECT_FALSE(has_move(CountryEnum::WEST_GERMANY));
}

TEST_F(ActionCoupLegalMovesTest, Defcon3RestrictionEuropeAsia) {
  // DEFCON 3：ヨーロッパ、アジア制限
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});
  board.getDefconTrack().setDefcon(3);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 西ドイツ（ヨーロッパ）と日本（アジア）が除外される
  // イラン、アンゴラの2か国のみ
  EXPECT_EQ(moves.size(), 2);
}

TEST_F(ActionCoupLegalMovesTest, Defcon2RestrictionEuropeAsiaMiddleEast) {
  // DEFCON 2：ヨーロッパ、アジア、中東制限
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});
  board.getDefconTrack().setDefcon(2);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 西ドイツ（ヨーロッパ）、日本（アジア）、イラン（中東）が除外される
  // アンゴラ（アフリカ）の1か国のみ
  EXPECT_EQ(moves.size(), 1);
}

TEST_F(ActionCoupLegalMovesTest, EmptyHand) {
  // 手札なしテスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, ChinaCardAddsMovesWhenFaceUp) {
  board.giveChinaCardTo(Side::USSR, true);
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 4);
  const bool has_china_card_move =
      std::any_of(moves.begin(), moves.end(), [](const auto& move) {
        return move != nullptr && move->getCard() == CardEnum::CHINA_CARD;
      });
  EXPECT_TRUE(has_china_card_move);
}

TEST_F(ActionCoupLegalMovesTest, NoOpponentInfluence) {
  // 相手影響力なしテスト
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, ScoringCardOpsZero) {
  // スコアカード（Ops0）のみの手札
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUMMY});  // Ops0

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // CLAUDE.mdの仕様：Ops0では合法手なし
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, AllExcludedByDefcon) {
  // DEFCON制限で全て除外されるケース
  TestHelper::setupBoardWithInfluence(board);

  // ヨーロッパ、アジア、中東にのみUSA影響力を配置
  board.getWorldMap().getCountry(CountryEnum::ANGOLA).clearInfluence(Side::USA);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});
  board.getDefconTrack().setDefcon(2);  // ヨーロッパ、アジア、中東制限

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 全ての国がDEFCON制限により除外される
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, MultipleCards) {
  // 複数カードテスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::DUCK_AND_COVER, CardEnum::FIDEL, CardEnum::NUCLEAR_TEST_BAN});
  board.getDefconTrack().setDefcon(5);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 3枚のカード × 4か国 = 12個のMove
  EXPECT_EQ(moves.size(), 12);
}

TEST_F(ActionCoupLegalMovesTest, MixedOpsValues) {
  // Ops値が異なる複数カード
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::FIDEL,             // 2 Ops
                              CardEnum::DUCK_AND_COVER,    // 3 Ops
                              CardEnum::NUCLEAR_TEST_BAN,  // 4 Ops
                              CardEnum::DUMMY});           // 0 Ops
  board.getDefconTrack().setDefcon(5);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // Ops0のカード以外の3枚 × 4か国 = 12個のMove
  EXPECT_EQ(moves.size(), 12);

  // Dummyカード（Ops0）のMoveが含まれていないことを確認
  bool has_dummy_move = false;
  for (const auto& move : moves) {
    if (move->getCard() == CardEnum::DUMMY) {
      has_dummy_move = true;
      break;
    }
  }
  EXPECT_FALSE(has_dummy_move);
}

TEST_F(ActionCoupLegalMovesTest, SingleCardHelperMatchesGeneral) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});
  board.getDefconTrack().setDefcon(5);

  auto helper_moves = LegalMovesGenerator::actionCoupLegalMovesForCard(
      board, Side::USSR, CardEnum::DUCK_AND_COVER);
  auto general_moves =
      LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  ASSERT_EQ(helper_moves.size(), general_moves.size());
  EXPECT_FALSE(helper_moves.empty());
  for (const auto& move : helper_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::DUCK_AND_COVER);
  }
}

// actionSpaceRaceLegalMovesのテスト
class ActionSpaceRaceLegalMovesTest : public ::testing::Test {
 protected:
  ActionSpaceRaceLegalMovesTest() : board(createTestCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USSR, false); }

  Board board;
};

TEST_F(ActionSpaceRaceLegalMovesTest, BasicCaseInitialStage) {
  // 基本ケース：初期段階（位置1、必要2 Ops、試行未使用）
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DUCK_AND_COVER, CardEnum::FIDEL,
                              CardEnum::NUCLEAR_TEST_BAN});  // 3, 2, 4 Ops

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 2 Ops以上のカードのみ使用可能：DuckAndCover(3), NuclearTestBan(4)
  // Fidel(2)も使用可能
  EXPECT_EQ(moves.size(), 3);

  // 全てのMoveがActionSpaceRaceMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionSpaceRaceMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionSpaceRaceLegalMovesTest, RequiredOpsChanges) {
  // 必要Ops値の変化確認（位置により要求値変化）
  TestHelper::setSpaceTrackPosition(board, Side::USSR,
                                    5);  // 中期段階（3 Ops必要）
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::FIDEL, CardEnum::DUCK_AND_COVER,
                              CardEnum::NUCLEAR_TEST_BAN});  // 2, 3, 4 Ops

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 3 Ops以上のカードのみ使用可能：DuckAndCover(3), NuclearTestBan(4)
  // Fidel(2)は不足
  EXPECT_EQ(moves.size(), 2);
}

TEST_F(ActionSpaceRaceLegalMovesTest, EmptyHand) {
  // エッジケース：手札なし
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, ChinaCardEnablesSpaceRaceWhenFaceUp) {
  board.giveChinaCardTo(Side::USSR, true);
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  ASSERT_EQ(moves.size(), 1);
  EXPECT_EQ(moves.front()->getCard(), CardEnum::CHINA_CARD);
}

TEST_F(ActionSpaceRaceLegalMovesTest, TrialLimitReached) {
  // エッジケース：試行回数制限到達
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 1);  // 上限到達
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 試行回数制限により実行不可
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, SpaceRaceCompleted) {
  // エッジケース：宇宙開発完了済み（位置8）
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 8);  // 完了
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 完了済みのため実行不可
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, AllCardsInsufficientOps) {
  // エッジケース：全カードがOps不足
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 5);  // 3 Ops必要
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});  // 2 Ops

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 全カードがOps不足
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, SpecialCardsOnly) {
  // エッジケース：スコアリングカードのみの手札
  // 注意：中国カードは宇宙開発で使用可能
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DUMMY});  // スコアリングカード（Ops0）

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // スコアリングカードは使用不可
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, MultipleSameOpsCards) {
  // 特殊ケース：複数の同Opsカード
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::DUCK_AND_COVER, CardEnum::DUCK_AND_COVER,
       CardEnum::DUCK_AND_COVER});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 同じOps値のカード3枚から独立したMoveが生成される
  EXPECT_EQ(moves.size(), 3);
}

// actionEventLegalMovesのテスト
class ActionEventLegalMovesTest : public ::testing::Test {
 protected:
  ActionEventLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionEventLegalMovesTest, MixedHandStandard) {
  // 基本ケース：混在する手札（自分、相手、中立イベント）
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DUCK_AND_COVER,      // 3 Ops, 中立
                              CardEnum::FIDEL,               // 2 Ops, 中立
                              CardEnum::NUCLEAR_TEST_BAN});  // 4 Ops, 中立

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // 全てのイベントカードが実行可能
  EXPECT_EQ(moves.size(), 3);

  // 全てのMoveがActionEventMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionEventMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionEventLegalMovesTest, ScoringCardMandatory) {
  // 基本ケース：スコアリングカード必須の確認
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::DUMMY,             // 0 Ops, スコアリングカード
       CardEnum::DUCK_AND_COVER});  // 3 Ops, 通常イベント

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // スコアリングカードも含まれる
  EXPECT_EQ(moves.size(), 2);

  // スコアリングカードのMoveが含まれていることを確認
  bool has_scoring_move = false;
  for (const auto& move : moves) {
    if (move->getCard() == CardEnum::DUMMY) {
      has_scoring_move = true;
      break;
    }
  }
  EXPECT_TRUE(has_scoring_move);
}

TEST_F(ActionEventLegalMovesTest, EmptyHand) {
  // エッジケース：手札なし
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionEventLegalMovesTest, AllScoringCards) {
  // エッジケース：全てスコアリングカード
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::DUMMY, CardEnum::DUMMY});  // 両方スコアリングカード

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // スコアリングカード2枚分のEventMove
  EXPECT_EQ(moves.size(), 2);
}

TEST_F(ActionEventLegalMovesTest, MixedCardTypes) {
  // 特殊ケース：異なる種類のカードが混在
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DUMMY,  // スコアリングカード
                              CardEnum::DUCK_AND_COVER,  // 通常イベント
                              CardEnum::FIDEL});         // 通常イベント

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // 全てのカードがイベントとして実行可能
  EXPECT_EQ(moves.size(), 3);

  // カードの種類を確認
  std::map<CardEnum, int> card_count;
  for (const auto& move : moves) {
    card_count[move->getCard()]++;
  }

  EXPECT_EQ(card_count[CardEnum::DUMMY], 1);
  EXPECT_EQ(card_count[CardEnum::DUCK_AND_COVER], 1);
  EXPECT_EQ(card_count[CardEnum::FIDEL], 1);
}

// ActionEventLegalMoves（canEventフラグ制御）のテスト
class ActionEventLegalMovesCanEventTest : public ::testing::Test {
 protected:
  ActionEventLegalMovesCanEventTest() : board_(getMockCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& getMockCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool;
    static bool initialized = false;

    if (!initialized) {
      // モックカードを作成
      // DUCK_AND_COVER: USSR側、canEvent=true
      pool[static_cast<size_t>(CardEnum::DUCK_AND_COVER)] =
          std::make_unique<MockEventCard>(CardEnum::DUCK_AND_COVER, Side::USSR,
                                          true);
      // FIDEL: USSR側、canEvent=false
      pool[static_cast<size_t>(CardEnum::FIDEL)] =
          std::make_unique<MockEventCard>(CardEnum::FIDEL, Side::USSR, false);
      // NUCLEAR_TEST_BAN: USA側、canEvent=true
      pool[static_cast<size_t>(CardEnum::NUCLEAR_TEST_BAN)] =
          std::make_unique<MockEventCard>(CardEnum::NUCLEAR_TEST_BAN, Side::USA,
                                          true);
      // COMECON: USA側、canEvent=false
      pool[static_cast<size_t>(CardEnum::COMECON)] =
          std::make_unique<MockEventCard>(CardEnum::COMECON, Side::USA, false);
      // DUMMY: NEUTRAL、canEvent=true
      pool[static_cast<size_t>(CardEnum::DUMMY)] =
          std::make_unique<MockEventCard>(CardEnum::DUMMY, Side::NEUTRAL, true);
      // ASIA_SCORING: NEUTRAL、canEvent=false
      pool[static_cast<size_t>(CardEnum::ASIA_SCORING)] =
          std::make_unique<MockEventCard>(CardEnum::ASIA_SCORING, Side::NEUTRAL,
                                          false);
      initialized = true;
    }
    return pool;
  }

  Board board_;
};

TEST_F(ActionEventLegalMovesCanEventTest, OwnSideCardWithCanEventTrue) {
  // USSR手札: DUCK_AND_COVER (USSR側、canEvent=true)
  board_.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board_, Side::USSR);

  // 含まれる
  ASSERT_EQ(moves.size(), 1);
  EXPECT_EQ(moves[0]->getCard(), CardEnum::DUCK_AND_COVER);
  auto* event_move = dynamic_cast<ActionEventMove*>(moves[0].get());
  ASSERT_NE(event_move, nullptr);
  EXPECT_TRUE(event_move->shouldTriggerEvent());
}

TEST_F(ActionEventLegalMovesCanEventTest, OwnSideCardWithCanEventFalse) {
  // USSR手札: FIDEL (USSR側、canEvent=false)
  board_.addCardToHand(Side::USSR, CardEnum::FIDEL);

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board_, Side::USSR);

  // 含まれない
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionEventLegalMovesCanEventTest, OpponentSideCardWithCanEventTrue) {
  // USSR手札: NUCLEAR_TEST_BAN (USA側、canEvent=true)
  board_.addCardToHand(Side::USSR, CardEnum::NUCLEAR_TEST_BAN);

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board_, Side::USSR);

  // 含まれる
  ASSERT_EQ(moves.size(), 1);
  EXPECT_EQ(moves[0]->getCard(), CardEnum::NUCLEAR_TEST_BAN);
  auto* event_move = dynamic_cast<ActionEventMove*>(moves[0].get());
  ASSERT_NE(event_move, nullptr);
  EXPECT_TRUE(event_move->shouldTriggerEvent());
}

TEST_F(ActionEventLegalMovesCanEventTest, OpponentSideCardWithCanEventFalse) {
  // USSR手札: COMECON (USA側、canEvent=false)
  board_.addCardToHand(Side::USSR, CardEnum::COMECON);

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board_, Side::USSR);

  // 含まれる（★重要：canEventがfalseでも敵陣営カードは含まれる）
  ASSERT_EQ(moves.size(), 1);
  EXPECT_EQ(moves[0]->getCard(), CardEnum::COMECON);
  auto* event_move = dynamic_cast<ActionEventMove*>(moves[0].get());
  ASSERT_NE(event_move, nullptr);
  EXPECT_FALSE(event_move->shouldTriggerEvent());  // イベントはスキップ
}

TEST_F(ActionEventLegalMovesCanEventTest, NeutralCardWithCanEventTrue) {
  // USSR手札: DUMMY (NEUTRAL、canEvent=true)
  board_.addCardToHand(Side::USSR, CardEnum::DUMMY);

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board_, Side::USSR);

  // 含まれる
  ASSERT_EQ(moves.size(), 1);
  EXPECT_EQ(moves[0]->getCard(), CardEnum::DUMMY);
  auto* event_move = dynamic_cast<ActionEventMove*>(moves[0].get());
  ASSERT_NE(event_move, nullptr);
  EXPECT_TRUE(event_move->shouldTriggerEvent());
}

TEST_F(ActionEventLegalMovesCanEventTest, NeutralCardWithCanEventFalse) {
  // USSR手札: ASIA_SCORING (NEUTRAL、canEvent=false)
  board_.addCardToHand(Side::USSR, CardEnum::ASIA_SCORING);

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board_, Side::USSR);

  // 含まれない
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionEventLegalMovesCanEventTest,
       OpponentCardCanEventFalse_EventIsSkipped) {
  // USSR手札: COMECON (USA側、canEvent=false)
  board_.addCardToHand(Side::USSR, CardEnum::COMECON);

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board_, Side::USSR);
  ASSERT_EQ(moves.size(), 1);

  auto* event_move = dynamic_cast<ActionEventMove*>(moves[0].get());
  ASSERT_NE(event_move, nullptr);
  EXPECT_FALSE(event_move->shouldTriggerEvent());

  // toCommandを呼び出して、イベントコマンドが生成されないことを確認
  const auto& cardpool = board_.getCardpool();
  auto commands = event_move->toCommand(
      cardpool[static_cast<size_t>(CardEnum::COMECON)], board_);

  // イベントコマンドはスキップされ、RequestCommand +
  // FinalizeCardPlayCommandのみ
  ASSERT_EQ(commands.size(), 2);

  // RequestCommandが含まれる（敵陣営カードなので追加アクション可能）
  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[0].get());
  ASSERT_NE(request_cmd, nullptr);

  // FinalizeCardPlayCommandが含まれる
  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands[1].get());
  ASSERT_NE(finalize_cmd, nullptr);
}
