#pragma once
#include <memory>
#include <vector>

#include "cards_enum.hpp"
#include "game_enums.hpp"

class Action;
class Game;

class Move {
 public:
  Move(MoveType MoveType, CardEnum card) : MoveType_{MoveType}, card_{card} {}
  virtual ~Move() = default;

  MoveType getMoveType() const { return MoveType_; }
  CardEnum getCard() const { return card_; }
  virtual std::unique_ptr<Action> toAction(const Game& game,
                                           Side side) const = 0;

 private:
  MoveType MoveType_;
  const CardEnum card_;
};

class PlaceInfluenceMove : public Move {
 public:
  PlaceInfluenceMove(
      CardEnum card,
      const std::vector<std::pair<CountryEnum, int>>& targetCountries)
      : Move{MoveType::PlaceInfluence, card},
        targetCountries_{targetCountries} {}

  std::unique_ptr<Action> toAction(const Game& game, Side side) const;
  const std::vector<std::pair<CountryEnum, int>>& getTargetCountries() const {
    return targetCountries_;
  }

 private:
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
};

class CoupMove : public Move {
 public:
  CoupMove(CardEnum card, CountryEnum targetCountry)
      : Move{MoveType::Coup, card}, targetCountry_{targetCountry} {}

  std::unique_ptr<Action> toAction(const Game& game, Side side) const;
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CountryEnum targetCountry_;
};

class SpaceRaceMove : public Move {
 public:
  SpaceRaceMove(CardEnum card) : Move{MoveType::SpaceRace, card} {}

  std::unique_ptr<Action> toAction(const Game& game, Side side) const;
};