#pragma once
#include "tsge/enums/cards_enum.hpp"
#include "tsge/game_state/card.hpp"

class DuckAndCover final : public Card {
 public:
  DuckAndCover()
      : Card(CardEnum::DUCK_AND_COVER, "DuckAndCover", 3, Side::USA,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class ChinaCard final : public Card {
 public:
  ChinaCard()
      : Card(CardEnum::CHINA_CARD, "ChinaCard", 4, Side::NEUTRAL,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class Fidel final : public Card {
 public:
  Fidel()
      : Card(CardEnum::FIDEL, "Fidel", 3, Side::USSR, WarPeriod::EARLY_WAR,
             true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class NuclearTestBan final : public Card {
 public:
  NuclearTestBan()
      : Card(CardEnum::NUCLEAR_TEST_BAN, "NuclearTestBan", 4, Side::NEUTRAL,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};
