#pragma once
#include "card_base.hpp"

class DuckAndCover : public CardBase {
 public:
  DuckAndCover() : CardBase(4, "DuckAndCover", 3, Side::USA, false) {}
  bool event(Game& game, Side side) override;
};

class Fidel : public CardBase {
 public:
  Fidel() : CardBase(8, "Fidel", 3, Side::USSR, true) {}
  bool event(Game& game, Side side) override;
};

class NuclearTestBan : public CardBase {
 public:
  NuclearTestBan() : CardBase(34, "NuclearTestBan", 4, Side::Neutral, false) {}
  bool event(Game& game, Side side) override;
};