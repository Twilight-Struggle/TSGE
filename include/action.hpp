#pragma once

#include <vector>

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
  Side side_;
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
  std::vector<std::pair<CountryEnum, int>> targetCountries_;
  std::set<CountryEnum> placeableCountries_;
};
