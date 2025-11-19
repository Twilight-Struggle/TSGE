#pragma once
// どこで: include/tsge/game_state/cards.hpp
// 何を: Twilight
// Struggleカード具象クラスを宣言し、Boardへのイベント橋渡しを担う なぜ:
// カードの仕様を型安全に実装し、テスト済みのイベントロジックを共有するため
#include "tsge/enums/cards_enum.hpp"
#include "tsge/game_state/card.hpp"

class RegionScoringCard : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  RegionScoringCard(CardEnum id, std::string&& name, WarPeriod warPeriod,
                    Region region)
      : Card(id, std::move(name), 0, Side::NEUTRAL, warPeriod, false),
        region_{region} {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;

 protected:
  Region region_;
};

class AsiaScoring final : public RegionScoringCard {
 public:
  AsiaScoring()
      : RegionScoringCard(CardEnum::ASIA_SCORING, "AsiaScoring",
                          WarPeriod::EARLY_WAR, Region::ASIA) {}
};

class EuropeScoring final : public RegionScoringCard {
 public:
  EuropeScoring()
      : RegionScoringCard(CardEnum::EUROPE_SCORING, "EuropeScoring",
                          WarPeriod::EARLY_WAR, Region::EUROPE) {}
};

class MiddleEastScoring final : public RegionScoringCard {
 public:
  MiddleEastScoring()
      : RegionScoringCard(CardEnum::MIDDLE_EAST_SCORING, "MiddleEastScoring",
                          WarPeriod::EARLY_WAR, Region::MIDDLE_EAST) {}
};

class CentralAmericaScoring final : public RegionScoringCard {
 public:
  CentralAmericaScoring()
      : RegionScoringCard(CardEnum::CENTRAL_AMERICA_SCORING,
                          "CentralAmericaScoring", WarPeriod::MID_WAR,
                          Region::CENTRAL_AMERICA) {}
};

class AfricaScoring final : public RegionScoringCard {
 public:
  AfricaScoring()
      : RegionScoringCard(CardEnum::AFRICA_SCORING, "AfricaScoring",
                          WarPeriod::MID_WAR, Region::AFRICA) {}
};

class SouthAmericaScoring final : public RegionScoringCard {
 public:
  SouthAmericaScoring()
      : RegionScoringCard(CardEnum::SOUTH_AMERICA_SCORING,
                          "SouthAmericaScoring", WarPeriod::MID_WAR,
                          Region::SOUTH_AMERICA) {}
};

class SoutheastAsiaScoring final : public Card {
 public:
  SoutheastAsiaScoring()
      : Card(CardEnum::SOUTHEAST_ASIA_SCORING, "SoutheastAsiaScoring", 0,
             Side::NEUTRAL, WarPeriod::MID_WAR, true) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class DuckAndCover final : public Card {
 public:
  DuckAndCover()
      : Card(CardEnum::DUCK_AND_COVER, "DuckAndCover", 3, Side::USA,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class ChinaCard final : public Card {
 public:
  ChinaCard()
      : Card(CardEnum::CHINA_CARD, "ChinaCard", 4, Side::NEUTRAL,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class Fidel final : public Card {
 public:
  Fidel()
      : Card(CardEnum::FIDEL, "Fidel", 3, Side::USSR, WarPeriod::EARLY_WAR,
             true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

class NuclearTestBan final : public Card {
 public:
  NuclearTestBan()
      : Card(CardEnum::NUCLEAR_TEST_BAN, "NuclearTestBan", 4, Side::NEUTRAL,
             WarPeriod::EARLY_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};

// 特殊な影響力配置カード
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
