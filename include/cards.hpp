#pragma once
#include "card_base.hpp"
#include "cards_enum.hpp"

class DuckAndCover : public CardBase {
 public:
  DuckAndCover()
      : CardBase(static_cast<int>(CardEnum::DuckAndCover), "DuckAndCover", 3,
                 Side::USA, false) {}
  bool event(Game& game, Side side) override;
};

class Fidel : public CardBase {
 public:
  Fidel()
      : CardBase(static_cast<int>(CardEnum::Fidel), "Fidel", 3, Side::USSR,
                 true) {}
  bool event(Game& game, Side side) override;
};

class NuclearTestBan : public CardBase {
 public:
  NuclearTestBan()
      : CardBase(static_cast<int>(CardEnum::NuclearTestBan), "NuclearTestBan",
                 4, Side::Neutral, false) {}
  bool event(Game& game, Side side) override;
};