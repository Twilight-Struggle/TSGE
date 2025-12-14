// どこで: include/tsge/actions/card_specific_moves.hpp
// 何を: カード固有のMove定義（De-Stalinization向け除去Move）を宣言
// なぜ: 共通Move群から個別カード実装を切り離し、拡張性と可読性を高めるため
#pragma once

#include <map>

#include "tsge/actions/move.hpp"

class DeStalinizationRemoveMove final : public Move {
 public:
  DeStalinizationRemoveMove(CardEnum card, Side side,
                            const std::map<CountryEnum, int>& targetCountries)
      : Move{card, side}, targetCountries_{targetCountries} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(const std::unique_ptr<Card>& card,
                                    const Board& board) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast =
        dynamic_cast<const DeStalinizationRemoveMove*>(&other);
    if (other_cast == nullptr) {
      return false;
    }
    return targetCountries_ == other_cast->targetCountries_;
  }

 private:
  const std::map<CountryEnum, int> targetCountries_;
};
