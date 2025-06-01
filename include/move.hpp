#pragma once
#include <memory>
#include <vector>

#include "card.hpp"
#include "cards_enum.hpp"
#include "command.hpp"
#include "game_enums.hpp"

class Game;

class Move {
 public:
  Move(CardEnum card) : card_{card} {}
  virtual ~Move() = default;

  CardEnum getCard() const { return card_; }
  virtual CommandPtr toCommand(const std::unique_ptr<Card>& card,
                               Side side) const = 0;

 private:
  const CardEnum card_;
};

class ActionPlaceInfluenceMove : public Move {
 public:
  ActionPlaceInfluenceMove(
      CardEnum card, std::vector<std::pair<CountryEnum, int>>&& targetCountries)
      : Move{card}, targetCountries_{std::move(targetCountries)} {}

  CommandPtr toCommand(const std::unique_ptr<Card>& card, Side side) const;
  const std::vector<std::pair<CountryEnum, int>>& getTargetCountries() const {
    return targetCountries_;
  }

 private:
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
};

class ActionCoupMove : public Move {
 public:
  ActionCoupMove(CardEnum card, CountryEnum targetCountry)
      : Move{card}, targetCountry_{targetCountry} {}

  CommandPtr toCommand(const std::unique_ptr<Card>& card, Side side) const;
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CountryEnum targetCountry_;
};

class ActionSpaceRaceMove : public Move {
 public:
  ActionSpaceRaceMove(CardEnum card) : Move{card} {}

  CommandPtr toCommand(const std::unique_ptr<Card>& card, Side side) const;
};

class ActionRealigmentMove : public Move {
 public:
  ActionRealigmentMove(CardEnum card, CountryEnum targetCountry)
      : Move{card}, targetCountry_{targetCountry} {}

  CommandPtr toCommand(const std::unique_ptr<Card>& card, Side side) const;
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CountryEnum targetCountry_;
};

class ActionEventMove : public Move {
 public:
  ActionEventMove(CardEnum card) : Move{card} {}

  CommandPtr toCommand(const std::unique_ptr<Card>& card, Side side) const;
};