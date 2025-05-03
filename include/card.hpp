#pragma once
#include <string>

#include "game_enums.hpp"

class Game;

class Card {
 public:
  Card(int id, std::string&& name, int ops, Side side, bool removedAfterEvent)
      : id_{id},
        name_{std::move(name)},
        ops_{ops},
        side_{side},
        removedAfterEvent_{removedAfterEvent} {}
  virtual ~Card() = default;
  virtual bool event(Game& game, Side side) = 0;

  int getOps() const { return ops_; }
  Side getSide() const { return side_; }

 protected:
  int id_;
  std::string name_;
  int ops_;
  Side side_;
  bool removedAfterEvent_;
};