#pragma once
#include "tsge/enums/cards_enum.hpp"
#include "tsge/game_state/card.hpp"

class DuckAndCover final : public Card {
 public:
  DuckAndCover()
      : Card(CardEnum::DuckAndCover, "DuckAndCover", 3, Side::USA,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class Fidel final : public Card {
 public:
  Fidel()
      : Card(CardEnum::Fidel, "Fidel", 3, Side::USSR, WarPeriod::EARLY_WAR,
             true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class NuclearTestBan final : public Card {
 public:
  NuclearTestBan()
      : Card(CardEnum::NuclearTestBan, "NuclearTestBan", 4, Side::NEUTRAL,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};