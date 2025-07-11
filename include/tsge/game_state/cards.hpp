#pragma once
#include "tsge/enums/cards_enum.hpp"
#include "tsge/game_state/card.hpp"

class DuckAndCover : public Card {
 public:
  DuckAndCover()
      : Card(static_cast<int>(CardEnum::DuckAndCover), "DuckAndCover", 3,
             Side::USA, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;

 private:
  class DefconBasedVpChangeCommand : public Command {
   public:
    explicit DefconBasedVpChangeCommand(Side side) : Command(side) {}
    void apply(Board& board) const override;
  };
};

class Fidel : public Card {
 public:
  Fidel()
      : Card(static_cast<int>(CardEnum::Fidel), "Fidel", 3, Side::USSR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
};

class NuclearTestBan : public Card {
 public:
  NuclearTestBan()
      : Card(static_cast<int>(CardEnum::NuclearTestBan), "NuclearTestBan", 4,
             Side::NEUTRAL, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;

 private:
  class DefconBasedVpChangeCommand : public Command {
   public:
    explicit DefconBasedVpChangeCommand(Side side) : Command(side) {}
    void apply(Board& board) const override;
  };
};