#pragma once
#include <map>
#include <memory>
#include <vector>

#include "card.hpp"
#include "cards_enum.hpp"
#include "command.hpp"
#include "game_enums.hpp"

class Game;

class Move {
 public:
  Move(CardEnum card, Side side) : card_{card}, side_{side} {}
  virtual ~Move() = default;

  CardEnum getCard() const { return card_; }
  Side getSide() const { return side_; }
  virtual std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const = 0;

 private:
  const CardEnum card_;
  const Side side_;
};

class ActionPlaceInfluenceMove : public Move {
 public:
  ActionPlaceInfluenceMove(CardEnum card, Side side,
                           const std::map<CountryEnum, int>& targetCountries)
      : Move{card, side}, targetCountries_{targetCountries} {}

  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;
  const std::map<CountryEnum, int>& getTargetCountries() const {
    return targetCountries_;
  }

 private:
  const std::map<CountryEnum, int> targetCountries_;
};

class ActionCoupMove : public Move {
 public:
  ActionCoupMove(CardEnum card, Side side, CountryEnum targetCountry)
      : Move{card, side}, targetCountry_{targetCountry} {}

  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CountryEnum targetCountry_;
};

class ActionSpaceRaceMove : public Move {
 public:
  ActionSpaceRaceMove(CardEnum card, Side side) : Move{card, side} {}

  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;
};

class ActionRealigmentMove : public Move {
 public:
  ActionRealigmentMove(CardEnum card, Side side, CountryEnum targetCountry)
      : Move{card, side}, targetCountry_{targetCountry} {}

  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;
  CountryEnum getTargetCountry() const { return targetCountry_; }

 private:
  const CountryEnum targetCountry_;
};

class ActionEventMove : public Move {
 public:
  ActionEventMove(CardEnum card, Side side) : Move{card, side} {}

  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;
};