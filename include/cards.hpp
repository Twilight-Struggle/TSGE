#pragma once
#include "card.hpp"
#include "cards_enum.hpp"

class DuckAndCover : public Card {
 public:
  DuckAndCover()
      : Card(static_cast<int>(CardEnum::DuckAndCover), "DuckAndCover", 3,
             Side::USA, false) {}
  bool event(Game& game, Side side) override;
};

class Fidel : public Card {
 public:
  Fidel()
      : Card(static_cast<int>(CardEnum::Fidel), "Fidel", 3, Side::USSR, true) {}
  bool event(Game& game, Side side) override;
};

class NuclearTestBan : public Card {
 public:
  NuclearTestBan()
      : Card(static_cast<int>(CardEnum::NuclearTestBan), "NuclearTestBan", 4,
             Side::NEUTRAL, false) {}
  bool event(Game& game, Side side) override;
};