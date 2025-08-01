#pragma once
#include <string>
#include <vector>

#include "tsge/actions/command.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"

class Board;

class Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  Card(CardEnum id, std::string&& name, int ops, Side side, WarPeriod warPeriod,
       bool removedAfterEvent)
      : id_{id},
        name_{std::move(name)},
        ops_{ops},
        side_{side},
        warPeriod_{warPeriod},
        removedAfterEvent_{removedAfterEvent} {}
  virtual ~Card() = default;
  Card(const Card&) = delete;
  Card& operator=(const Card&) = delete;
  Card(Card&&) = delete;
  Card& operator=(Card&&) = delete;

  [[nodiscard]]
  virtual std::vector<CommandPtr> event(Side side) const = 0;
  [[nodiscard]]
  virtual bool canEvent(const Board& board) const = 0;
  [[nodiscard]]
  int getOps() const {
    return ops_;
  }
  [[nodiscard]]
  Side getSide() const {
    return side_;
  }
  [[nodiscard]]
  WarPeriod getWarPeriod() const {
    return warPeriod_;
  }

 protected:
  CardEnum id_;
  std::string name_;
  int ops_;
  Side side_;
  WarPeriod warPeriod_;
  bool removedAfterEvent_;
};