#pragma once
#include <memory>
#include <vector>

#include "card.hpp"
#include "cards_enum.hpp"
#include "game_enums.hpp"

class Command;
class Game;

class Move {
 public:
  Move(MoveType MoveType, CardEnum card) : MoveType_{MoveType}, card_{card} {}
  virtual ~Move() = default;

  MoveType getMoveType() const { return MoveType_; }
  CardEnum getCard() const { return card_; }
  virtual std::unique_ptr<const Command> toCommand(
      const std::unique_ptr<Card>& card, Side side) const = 0;

 private:
  MoveType MoveType_;
  const CardEnum card_;
};

class PlaceInfluenceMove : public Move {
 public:
  PlaceInfluenceMove(CardEnum card,
                     std::vector<std::pair<CountryEnum, int>>&& targetCountries)
      : Move{MoveType::PLACE_INFLUENCE, card},
        targetCountries_{std::move(targetCountries)} {}

  std::unique_ptr<const Command> toCommand(const std::unique_ptr<Card>& card,
                                           Side side) const;
  const std::vector<std::pair<CountryEnum, int>>& getTargetCountries() const {
    return targetCountries_;
  }

 private:
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
};

class CoupMove : public Move {
 public:
  CoupMove(CardEnum card, CountryEnum targetCountry)
      : Move{MoveType::COUP, card}, targetCountry_{targetCountry} {}

  std::unique_ptr<const Command> toCommand(const std::unique_ptr<Card>& card,
                                           Side side) const;
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CountryEnum targetCountry_;
};

class SpaceRaceMove : public Move {
 public:
  SpaceRaceMove(CardEnum card) : Move{MoveType::SPACE_RACE, card} {}

  std::unique_ptr<const Command> toCommand(const std::unique_ptr<Card>& card,
                                           Side side) const;
};

class RealigmentMove : public Move {
 public:
  RealigmentMove(CardEnum card, CountryEnum targetCountry)
      : Move{MoveType::REALIGNMENT, card}, targetCountry_{targetCountry} {}

  std::unique_ptr<const Command> toCommand(const std::unique_ptr<Card>& card,
                                           Side side) const;
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CountryEnum targetCountry_;
};

class EventMove : public Move {
 public:
  EventMove(CardEnum card) : Move{MoveType::EVENT, card} {}

  std::unique_ptr<const Command> toCommand(const std::unique_ptr<Card>& card,
                                           Side side) const;
};