#pragma once

#include <vector>

#include "game_enums.hpp"

class Game;

class Action {
 public:
  Action(MoveType type, Side side, int opeValue)
      : type_{type}, side_{side}, opeValue_{opeValue} {};
  virtual ~Action() = default;
  virtual bool execute(Game& game) const = 0;

  MoveType getType() const { return type_; }

 protected:
  const Side side_;
  int opeValue_;

 private:
  MoveType type_;
};

class PlaceInfluence : public Action {
 public:
  PlaceInfluence(
      Side side, int opeValue,
      const std::vector<std::pair<CountryEnum, int>>& targetCountries)
      : Action{MoveType::PLACE_INFLUENCE, side, opeValue},
        targetCountries_{targetCountries} {};

  bool execute(Game& game) const override;

 private:
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
};

class Realigment : public Action {
 public:
  Realigment(Side side, CountryEnum targetCountry)
      : Action{MoveType::REALIGNMENT, side, 1},
        targetCountry_{targetCountry} {};

  bool execute(Game& game) const override;

 private:
  const CountryEnum targetCountry_;
};

class Coup : public Action {
 public:
  Coup(Side side, int opeValue, CountryEnum targetCountry)
      : Action{MoveType::COUP, side, opeValue},
        targetCountry_{targetCountry} {};

  bool execute(Game& game) const override;

 private:
  const CountryEnum targetCountry_;
};

class SpaceRace : public Action {
 public:
  SpaceRace(Side side, int opeValue)
      : Action{MoveType::SPACE_RACE, side, opeValue} {};

  bool execute(Game& game) const override;
};
