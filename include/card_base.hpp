#pragma once
#include <string>

#include "game_enums.hpp"

class Game;

class CardBase {
 public:
  CardBase(int id, const std::string& name, int ops, Side side)
      : id_{id}, name_{name}, ops_{ops}, side_{side} {}
  virtual ~CardBase() = default;
  virtual bool event(Game& game, Side side) = 0;

 protected:
  int id_;
  std::string name_;
  int ops_;
  Side side_;
};