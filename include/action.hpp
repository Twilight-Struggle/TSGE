#pragma once

#include <vector>

#include "game.hpp"
#include "game_enums.hpp"

class Action {
 public:
  Action(ActionType type, Side side, int opeValue)
      : type_{type}, side_{side}, opeValue_{opeValue} {};
  virtual ~Action() = default;
  virtual bool execute(Game& game) = 0;

  ActionType getType() const { return type_; }

 protected:
  const Side side_;
  int opeValue_;

 private:
  ActionType type_;
};

class PlaceInfluence : public Action {
 public:
  PlaceInfluence(
      Side side, int opeValue,
      const std::vector<std::pair<CountryEnum, int>>& targetCountries)
      : Action{ActionType::PlaceInfluence, side, opeValue},
        targetCountries_{targetCountries} {};

  bool execute(Game& game) override;

 private:
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
};

class Realigment : public Action {
 public:
  Realigment(Side side, CountryEnum targetCountry)
      : Action{ActionType::Realignment, side, 1},
        targetCountry_{targetCountry} {};

  bool execute(Game& game) override;

 private:
  const CountryEnum targetCountry_;
};

class Coup : public Action {
 public:
  Coup(Side side, int opeValue, CountryEnum targetCountry)
      : Action{ActionType::Coup, side, opeValue},
        targetCountry_{targetCountry} {};

  bool execute(Game& game) override;

 private:
  const CountryEnum targetCountry_;
};

class SpaceRace : public Action {
 public:
  SpaceRace(Side side, int opeValue)
      : Action{ActionType::SpaceRace, side, opeValue} {};

  bool execute(Game& game) override;
};
