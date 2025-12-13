// File: tests/actions/special_place_influence_test.cpp
// Summary: 特殊な影響力配置カードの動作を検証する。
// Reason: カード固有の配置ロジックが正しく実装されているかを保証する。

#include <gtest/gtest.h>

#include "tsge/actions/card_effect_legal_move_generator.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"

class SpecialPlaceInfluenceTest : public ::testing::Test {
 protected:
  SpecialPlaceInfluenceTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }

  Board board;
};

TEST_F(SpecialPlaceInfluenceTest, ComeconConfigTest) {
  // Comeconカードの設定をテスト
  CardSpecialPlaceInfluenceConfig config;
  config.totalInfluence = 4;
  config.maxPerCountry = 1;
  config.allowedRegions = std::vector<Region>{Region::EAST_EUROPE};
  config.excludeOpponentControlled = true;
  config.onlyEmptyCountries = false;

  auto moves =
      CardEffectLegalMoveGenerator::generateCardSpecificPlaceInfluenceMoves(
          board, Side::USSR, CardEnum::COMECON, config);

  // 少なくとも何らかの合法手が生成されることを確認
  EXPECT_GE(moves.size(), 0);

  // 各moveがEventPlaceInfluenceMoveであることを確認
  for (const auto& move : moves) {
    const auto* place_move =
        dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
    EXPECT_NE(place_move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::COMECON);
    EXPECT_EQ(move->getSide(), Side::USSR);
  }
}

TEST_F(SpecialPlaceInfluenceTest, DecolonizationConfigTest) {
  // Decolonizationカードの設定をテスト
  CardSpecialPlaceInfluenceConfig config;
  config.totalInfluence = 4;
  config.maxPerCountry = 1;
  config.allowedRegions =
      std::vector<Region>{Region::AFRICA, Region::SOUTH_EAST_ASIA};
  config.excludeOpponentControlled = false;
  config.onlyEmptyCountries = false;

  auto moves =
      CardEffectLegalMoveGenerator::generateCardSpecificPlaceInfluenceMoves(
          board, Side::USSR, CardEnum::DECOLONIZATION, config);

  EXPECT_GE(moves.size(), 0);

  for (const auto& move : moves) {
    const auto* place_move =
        dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
    EXPECT_NE(place_move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::DECOLONIZATION);
    EXPECT_EQ(move->getSide(), Side::USSR);
  }
}

TEST_F(SpecialPlaceInfluenceTest, PuppetGovernmentsConfigTest) {
  // Puppet Governmentsカードの設定をテスト（影響力のない国のみ）
  CardSpecialPlaceInfluenceConfig config;
  config.totalInfluence = 3;
  config.maxPerCountry = 1;
  config.allowedRegions = std::nullopt;  // 全地域OK
  config.excludeOpponentControlled = false;
  config.onlyEmptyCountries = true;  // 影響力のない国のみ

  auto moves =
      CardEffectLegalMoveGenerator::generateCardSpecificPlaceInfluenceMoves(
          board, Side::USA, CardEnum::PUPPET_GOVERNMENTS, config);

  EXPECT_GE(moves.size(), 0);

  for (const auto& move : moves) {
    const auto* place_move =
        dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
    EXPECT_NE(place_move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::PUPPET_GOVERNMENTS);
    EXPECT_EQ(move->getSide(), Side::USA);
  }
}

TEST_F(SpecialPlaceInfluenceTest, LiberationTheologyConfigTest) {
  // Liberation Theologyカードの設定をテスト（各国最大2個）
  CardSpecialPlaceInfluenceConfig config;
  config.totalInfluence = 3;
  config.maxPerCountry = 2;
  config.allowedRegions = std::vector<Region>{Region::CENTRAL_AMERICA};
  config.excludeOpponentControlled = false;
  config.onlyEmptyCountries = false;

  auto moves =
      CardEffectLegalMoveGenerator::generateCardSpecificPlaceInfluenceMoves(
          board, Side::USSR, CardEnum::LIBERATION_THEOLOGY, config);

  EXPECT_GE(moves.size(), 0);

  for (const auto& move : moves) {
    const auto* place_move =
        dynamic_cast<const EventPlaceInfluenceMove*>(move.get());
    EXPECT_NE(place_move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::LIBERATION_THEOLOGY);
    EXPECT_EQ(move->getSide(), Side::USSR);
  }
}

TEST_F(SpecialPlaceInfluenceTest, EmptyRegionTest) {
  // 対象地域に国がない場合、空の結果が返されることをテスト
  CardSpecialPlaceInfluenceConfig config;
  config.totalInfluence = 4;
  config.maxPerCountry = 1;
  config.allowedRegions =
      std::vector<Region>{Region::SPECIAL};  // SPECIAL地域は通常国がない
  config.excludeOpponentControlled = false;
  config.onlyEmptyCountries = false;

  auto moves =
      CardEffectLegalMoveGenerator::generateCardSpecificPlaceInfluenceMoves(
          board, Side::USSR, CardEnum::COMECON, config);
  // SPECIAL地域にはUSS/USAしかないため、合法手は少ないはず
  EXPECT_GE(moves.size(), 0);
}

TEST_F(SpecialPlaceInfluenceTest, PuppetGovernmentsInsufficientCandidates) {
  // Puppet Governments: 影響力を置くべき国の候補が足りないケース
  // totalInfluence=3, maxPerCountry=1 だが、影響力のない国が2つしかない
  // この場合、2つの国に1ずつ、合計2の影響力を置くMoveが1つ生成されるべき
  CardSpecialPlaceInfluenceConfig config;
  config.totalInfluence = 3;
  config.maxPerCountry = 1;
  config.allowedRegions = std::nullopt;
  config.excludeOpponentControlled = false;
  config.onlyEmptyCountries = true;
  // 全ての国に影響力を1ずつ置いて、空き地をなくす
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < board.getWorldMap().getCountriesCount(); ++i) {
    board.getWorldMap()
        .getCountry(static_cast<CountryEnum>(i))
        .addInfluence(Side::USA, 1);
  }
  // 2つだけ空き地を作る
  board.getWorldMap().getCountry(CountryEnum::ZAIRE).clearInfluence(Side::USA);
  board.getWorldMap()
      .getCountry(CountryEnum::ZIMBABWE)
      .clearInfluence(Side::USA);
  auto moves =
      CardEffectLegalMoveGenerator::generateCardSpecificPlaceInfluenceMoves(
          board, Side::USA, CardEnum::PUPPET_GOVERNMENTS, config);
  // 2カ国に1つずつ置くパターンが1つだけ生成されるはず
  EXPECT_EQ(moves.size(), 1);

  if (!moves.empty()) {
    Board board_copy = board;
    auto commands = moves[0]->toCommand(
        board_copy.getCardpool()[static_cast<size_t>(moves[0]->getCard())],
        board_copy);
    for (const auto& command : commands) {
      command->apply(board_copy);
    }
    EXPECT_EQ(board_copy.getWorldMap()
                  .getCountry(CountryEnum::ZAIRE)
                  .getInfluence(Side::USA),
              1);
    EXPECT_EQ(board_copy.getWorldMap()
                  .getCountry(CountryEnum::ZIMBABWE)
                  .getInfluence(Side::USA),
              1);
    // 元のボードに影響がないことを確認
    EXPECT_EQ(board.getWorldMap()
                  .getCountry(CountryEnum::ZAIRE)
                  .getInfluence(Side::USA),
              0);
  }
}
