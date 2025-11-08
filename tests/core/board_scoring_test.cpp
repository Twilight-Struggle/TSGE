// tests/core/board_scoring_test.cpp
// Board::scoreRegionの得点仕様をE2Eで確認し、Presence/Domination/Controlの条件崩れを防ぐ。

#include <gtest/gtest.h>

#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"

class DummyCard : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  DummyCard(CardEnum id, WarPeriod warPeriod)
      : Card(id, "DummyCard", 3, Side::NEUTRAL, warPeriod, false) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side /*side*/) const override {
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& /*board*/) const override {
    return true;
  }
};

class BoardScoringTest : public ::testing::Test {
 protected:
  void SetUp() override {
    cardpool_ = std::make_unique<std::array<std::unique_ptr<Card>, 111>>();
    for (int i = 0; i < 50; ++i) {
      (*cardpool_)[i] =
          std::make_unique<DummyCard>(CardEnum::DUMMY, WarPeriod::EARLY_WAR);
    }
    board_ = std::make_unique<Board>(*cardpool_);
    board_->getDeck().addEarlyWarCards();
  }

  void neutralizeRegion(Region region) {
    auto& world_map = board_->getWorldMap();
    for (const auto& snapshot : world_map.countriesInRegion(region)) {
      auto& country = world_map.getCountry(snapshot.getId());
      country.clearInfluence(Side::USSR);
      country.clearInfluence(Side::USA);
    }
  }

  void forceControl(CountryEnum countryEnum, Side side) {
    auto& country = board_->getWorldMap().getCountry(countryEnum);
    country.clearInfluence(Side::USSR);
    country.clearInfluence(Side::USA);
    if (side == Side::NEUTRAL) {
      return;
    }
    country.addInfluence(side, country.getStability());
  }

  void controlBattlegrounds(Region region, Side side) {
    auto& world_map = board_->getWorldMap();
    for (const auto& snapshot : world_map.countriesInRegion(region)) {
      auto& country = world_map.getCountry(snapshot.getId());
      country.clearInfluence(Side::USSR);
      country.clearInfluence(Side::USA);
      if (country.isBattleground() && side != Side::NEUTRAL) {
        country.addInfluence(side, country.getStability());
      }
    }
  }

  std::unique_ptr<std::array<std::unique_ptr<Card>, 111>> cardpool_;
  std::unique_ptr<Board> board_;
};

TEST_F(BoardScoringTest, PresenceAndDominationScoringWorks) {
  neutralizeRegion(Region::CENTRAL_AMERICA);
  forceControl(CountryEnum::CUBA, Side::USSR);
  EXPECT_EQ(board_->scoreRegion(Region::CENTRAL_AMERICA, false), 3);

  neutralizeRegion(Region::ASIA);
  forceControl(CountryEnum::THAILAND, Side::USSR);
  forceControl(CountryEnum::INDIA, Side::USSR);
  forceControl(CountryEnum::VIETNAM, Side::USSR);
  forceControl(CountryEnum::JAPAN, Side::USA);
  EXPECT_EQ(board_->scoreRegion(Region::ASIA, false), 5);
}

TEST_F(BoardScoringTest, EuropeControlAwardsThousandPoints) {
  controlBattlegrounds(Region::EUROPE, Side::USSR);

  auto& world_map = board_->getWorldMap();
  int battleground_bonus = 0;
  int adjacency_bonus = 0;
  for (const auto& snapshot : world_map.countriesInRegion(Region::EUROPE)) {
    const auto& country = world_map.getCountry(snapshot.getId());
    if (country.getControlSide() != Side::USSR) {
      continue;
    }
    if (country.isBattleground()) {
      ++battleground_bonus;
    }
    for (const auto adjacent : country.getAdjacentCountries()) {
      if (adjacent == CountryEnum::USA) {
        ++adjacency_bonus;
        break;
      }
    }
  }

  EXPECT_EQ(board_->scoreRegion(Region::EUROPE, false),
            1000 + battleground_bonus + adjacency_bonus);
}
