#pragma once
#include <vector>

#include "cards_enum.hpp"
#include "game_enums.hpp"

class Move {
 public:
  Move(MoveType MoveType) : MoveType_{MoveType} {}
  virtual ~Move() = default;

  MoveType getMoveType() const { return MoveType_; }

 private:
  MoveType MoveType_;
};

class PlaceInfluenceMove : public Move {
 public:
  PlaceInfluenceMove(
      CardEnum card,
      const std::vector<std::pair<CountryEnum, int>>& targetCountries)
      : Move{MoveType::PlaceInfluence},
        card_{card},
        targetCountries_{targetCountries} {}

  CardEnum getCard() const { return card_; }
  const std::vector<std::pair<CountryEnum, int>>& getTargetCountries() const {
    return targetCountries_;
  }

 private:
  const CardEnum card_;
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
};

class CoupMove : public Move {
 public:
  CoupMove(CardEnum card, CountryEnum targetCountry)
      : Move{MoveType::Coup}, card_{card}, targetCountry_{targetCountry} {}

  CardEnum getCard() const { return card_; }
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CardEnum card_;
  const CountryEnum targetCountry_;
};

class SpaceRaceMove : public Move {
 public:
  SpaceRaceMove(CardEnum card) : Move{MoveType::SpaceRace}, card_{card} {}

  CardEnum getCard() const { return card_; }

 private:
  const CardEnum card_;
};