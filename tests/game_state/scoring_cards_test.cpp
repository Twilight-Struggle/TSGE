// File: tests/game_state/scoring_cards_test.cpp
// Summary: スコアリングカードの動作を検証する。
// Reason:
// スコアリングカードの実装が正しく地域スコアリングを行うことを保証する。

#include <gtest/gtest.h>

#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

class ScoringCardTest : public ::testing::Test {
 protected:
  ScoringCardTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }

  void neutralizeRegion(Region region) {
    auto& world_map = board.getWorldMap();
    for (const auto& snapshot : world_map.countriesInRegion(region)) {
      auto& country = world_map.getCountry(snapshot.getId());
      country.clearInfluence(Side::USSR);
      country.clearInfluence(Side::USA);
    }
  }

  void setControl(CountryEnum countryEnum, Side side) {
    auto& country = board.getWorldMap().getCountry(countryEnum);
    country.clearInfluence(Side::USSR);
    country.clearInfluence(Side::USA);
    if (side == Side::NEUTRAL) {
      return;
    }
    country.addInfluence(side, country.getStability());
  }

  Board board;
};

TEST_F(ScoringCardTest, AsiaScoringReflectsBoardScoreRegion) {
  neutralizeRegion(Region::ASIA);
  setControl(CountryEnum::THAILAND, Side::USSR);
  setControl(CountryEnum::INDIA, Side::USSR);
  setControl(CountryEnum::JAPAN, Side::USA);

  AsiaScoring sut;
  const int expected = board.scoreRegion(Region::ASIA, false);
  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);
  commands.front()->apply(board);

  auto& states = board.getStates();
  ASSERT_FALSE(states.empty());
  auto& vp_command = std::get<CommandPtr>(states.back());
  vp_command->apply(board);
  EXPECT_EQ(board.getVp(), expected);
}

TEST_F(ScoringCardTest, SoutheastAsiaScoringUsesCountryWeights) {
  neutralizeRegion(Region::ASIA);
  setControl(CountryEnum::THAILAND, Side::USSR);      // 2 VP
  setControl(CountryEnum::VIETNAM, Side::USSR);       // +1 VP
  setControl(CountryEnum::PHILIPPINES, Side::USA);    // -1 VP
  setControl(CountryEnum::MALAYSIA, Side::NEUTRAL);   // 0 VP
  setControl(CountryEnum::INDONESIA, Side::NEUTRAL);  // 0 VP
  setControl(CountryEnum::BURMA, Side::NEUTRAL);      // 0 VP
  setControl(CountryEnum::LAOS, Side::NEUTRAL);       // 0 VP

  SoutheastAsiaScoring sut;
  auto commands = sut.event(Side::USSR, board);
  ASSERT_EQ(commands.size(), 1);
  commands.front()->apply(board);

  auto& states = board.getStates();
  ASSERT_FALSE(states.empty());
  auto& vp_command = std::get<CommandPtr>(states.back());
  vp_command->apply(board);
  EXPECT_EQ(board.getVp(), 2);
}
