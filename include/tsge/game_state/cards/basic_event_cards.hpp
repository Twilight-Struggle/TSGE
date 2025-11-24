#pragma once
// どこで: include/tsge/game_state/cards/basic_event_cards.hpp
// 何を: 基本的なイベントカードの宣言を集約
// なぜ: カード実装を分類し、可読性と保守性を向上させるため

#include "tsge/enums/cards_enum.hpp"
#include "tsge/game_state/card.hpp"

class DuckAndCover final : public Card {
 public:
  DuckAndCover()
      : Card(CardEnum::DUCK_AND_COVER, "Duck and Cover", 3, Side::USA,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class ChinaCard final : public Card {
 public:
  ChinaCard()
      : Card(CardEnum::CHINA_CARD, "China Card", 4, Side::NEUTRAL,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class Fidel final : public Card {
 public:
  Fidel()
      : Card(CardEnum::FIDEL, "Fidel", 2, Side::USSR, WarPeriod::EARLY_WAR,
             true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class NuclearTestBan final : public Card {
 public:
  NuclearTestBan()
      : Card(CardEnum::NUCLEAR_TEST_BAN, "Nuclear Test Ban", 4, Side::NEUTRAL,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class Comecon final : public Card {
 public:
  Comecon()
      : Card(CardEnum::COMECON, "Comecon", 3, Side::USSR, WarPeriod::EARLY_WAR,
             true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class Decolonization final : public Card {
 public:
  Decolonization()
      : Card(CardEnum::DECOLONIZATION, "Decolonization", 2, Side::USSR,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class DestaLinization final : public Card {
 public:
  DestaLinization()
      : Card(CardEnum::DE_STALINIZATION, "De-Stalinization", 3, Side::USSR,
             WarPeriod::EARLY_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class ColonialRearGuards final : public Card {
 public:
  ColonialRearGuards()
      : Card(CardEnum::COLONIAL_REAR_GUARDS, "Colonial Rear Guards", 2,
             Side::USA, WarPeriod::MID_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class PuppetGovernments final : public Card {
 public:
  PuppetGovernments()
      : Card(CardEnum::PUPPET_GOVERNMENTS, "Puppet Governments", 2, Side::USA,
             WarPeriod::MID_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class OASFounded final : public Card {
 public:
  OASFounded()
      : Card(CardEnum::OAS_FOUNDED, "OAS Founded", 1, Side::USA,
             WarPeriod::MID_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class LiberationTheology final : public Card {
 public:
  LiberationTheology()
      : Card(CardEnum::LIBERATION_THEOLOGY, "Liberation Theology", 2,
             Side::USSR, WarPeriod::MID_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class WarsawPactFormed final : public Card {
 public:
  WarsawPactFormed()
      : Card(CardEnum::WARSAW_PACT_FORMED, "Warsaw Pact Formed", 3, Side::USSR,
             WarPeriod::EARLY_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class MarshallPlan final : public Card {
 public:
  MarshallPlan()
      : Card(CardEnum::MARSHALL_PLAN, "Marshall Plan", 4, Side::USA,
             WarPeriod::EARLY_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class UssuriRiverSkirmish final : public Card {
 public:
  UssuriRiverSkirmish()
      : Card(CardEnum::USSURI_RIVER_SKIRMISH, "Ussuri River Skirmish", 3,
             Side::USA, WarPeriod::MID_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class TheReformer final : public Card {
 public:
  TheReformer()
      : Card(CardEnum::REFORMER, "The Reformer", 3, Side::USSR,
             WarPeriod::LATE_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class SpecialRelationship final : public Card {
 public:
  SpecialRelationship()
      : Card(CardEnum::SPECIAL_RELATIONSHIP, "Special Relationship", 2,
             Side::USA, WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class SouthAfricanUnrest final : public Card {
 public:
  SouthAfricanUnrest()
      : Card(CardEnum::SOUTH_AFRICAN_UNREST, "South African Unrest", 2,
             Side::USSR, WarPeriod::MID_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class Junta final : public Card {
 public:
  Junta()
      : Card(CardEnum::JUNTA, "Junta", 2, Side::NEUTRAL, WarPeriod::MID_WAR,
             false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};
