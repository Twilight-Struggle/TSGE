#pragma once

#include <vector>

#include "game_enums.hpp"
#include "world_map.hpp"

enum class ActionType { Coup, Realignment, PlaceInfluence, Event, SpaceRace };

class Action {
 public:
  Action(const ActionType type, const Side side, const int opeValue)
      : type_{type}, side_{side}, opeValue_{opeValue} {};
  virtual ~Action() = default;
  virtual bool execute(WorldMap& worldMap) = 0;

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
      const Side side, const int opeValue,
      const std::vector<std::pair<CountryEnum, int>>& targetCountries,
      const std::set<CountryEnum>& placeableCountries)
      : Action{ActionType::PlaceInfluence, side, opeValue},
        targetCountries_{targetCountries},
        placeableCountries_{placeableCountries} {};

  bool execute(WorldMap& worldMap) override;

 private:
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
  const std::set<CountryEnum> placeableCountries_;
};

class Realigment : public Action {
 public:
  Realigment(const Side side, const CountryEnum targetCountry)
      : Action{ActionType::Realignment, side, 1},
        targetCountry_{targetCountry} {};

  bool execute(WorldMap& worldMap) override;

 private:
  const CountryEnum targetCountry_;
};

class Coup : public Action {
 public:
  Coup(const Side side, const int opeValue, const CountryEnum targetCountry)
      : Action{ActionType::Coup, side, opeValue},
        targetCountry_{targetCountry} {};

  bool execute(WorldMap& worldMap) override;

 private:
  const CountryEnum targetCountry_;
};
