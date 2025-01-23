#pragma once

#include <vector>

#include "board.hpp"

enum class ActionType { Coup, Realignment, PlaceInfluence, Event, SpaceRace };

class Action {
 public:
  Action(const ActionType type) : type_{type} {};
  virtual ~Action() = default;

  ActionType getType() const { return type_; }

 private:
  ActionType type_;
};

class PlaceInfluence : public Action {
 public:
  PlaceInfluence(
      const std::vector<std::pair<CountryEnum, int>>& targetCountries)
      : Action{ActionType::PlaceInfluence}, targetCountries_(targetCountries){};

  void execute(Board& country, const Side side);

 private:
  std::vector<std::pair<CountryEnum, int>> targetCountries_;
};
