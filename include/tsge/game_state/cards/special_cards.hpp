#pragma once
// どこで: include/tsge/game_state/cards/special_cards.hpp
// 何を: 特殊なカードの宣言を集約
// なぜ: カード実装を分類し、可読性と保守性を向上させるため

#include "tsge/enums/cards_enum.hpp"
#include "tsge/game_state/card.hpp"

class DeStainization final : public Card {
 public:
  DeStainization()
      : Card(CardEnum::DE_STALINIZATION, "De-Stalinization", 3, Side::USSR,
             WarPeriod::EARLY_WAR, true) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};
