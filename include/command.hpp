#pragma once

#include <memory>
#include <vector>

#include "game_enums.hpp"

class Board;

class Command {
 public:
  Command(MoveType type, Side side, int opeValue)
      : type_{type}, side_{side}, opeValue_{opeValue} {};
  virtual ~Command() = default;
  virtual bool apply(Board& game) const = 0;

  MoveType getType() const { return type_; }

 protected:
  const Side side_;
  int opeValue_;

 private:
  MoveType type_;
};

class PlaceInfluence : public Command {
 public:
  PlaceInfluence(
      Side side, int opeValue,
      const std::vector<std::pair<CountryEnum, int>>& targetCountries)
      : Command{MoveType::PLACE_INFLUENCE, side, opeValue},
        targetCountries_{targetCountries} {};

  bool apply(Board& game) const override;

 private:
  const std::vector<std::pair<CountryEnum, int>> targetCountries_;
};

class Realigment : public Command {
 public:
  Realigment(Side side, CountryEnum targetCountry)
      : Command{MoveType::REALIGNMENT, side, 1},
        targetCountry_{targetCountry} {};

  bool apply(Board& game) const override;

 private:
  const CountryEnum targetCountry_;
};

class Coup : public Command {
 public:
  Coup(Side side, int opeValue, CountryEnum targetCountry)
      : Command{MoveType::COUP, side, opeValue},
        targetCountry_{targetCountry} {};

  bool apply(Board& game) const override;

 private:
  const CountryEnum targetCountry_;
};

class SpaceRace : public Command {
 public:
  SpaceRace(Side side, int opeValue)
      : Command{MoveType::SPACE_RACE, side, opeValue} {};

  bool apply(Board& game) const override;
};

using CommandPtr = std::shared_ptr<Command>;

class Request : public Command {
 public:
  Side waitingForSide;

  bool apply(Board&) const override { return true; }
};