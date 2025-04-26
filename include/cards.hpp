#pragma once
#include "card_base.hpp"

class NuclearTestBan : public CardBase {
 public:
  NuclearTestBan() : CardBase(34, "NuclearTestBan", 4, Side::Neutral) {}
  bool event(Game& game, Side side) override;
};