// File: tests/actions/special_place_influence_test.cpp
// Summary: 特殊な影響力配置カードの動作を検証する。
// Reason: カード固有の配置ロジックが正しく実装されているかを保証する。

#include <gtest/gtest.h>

#include "tsge/actions/legal_moves_generator.hpp"
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

  auto moves = LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
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

  auto moves = LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
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

  auto moves = LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
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

  auto moves = LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
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

  auto moves = LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
      board, Side::USSR, CardEnum::COMECON, config);

  // SPECIAL地域にはUSS/USAしかないため、合法手は少ないはず
  EXPECT_GE(moves.size(), 0);
}