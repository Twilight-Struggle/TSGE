// どこで: src/actions/card_specific_moves.cpp
// 何を: カード固有Moveの実装（現状De-Stalinization用）を提供
// なぜ:
// 共通Move実装から個別カードロジックを分離し、増加する派生Moveを管理しやすくするため
#include "tsge/actions/card_specific_moves.hpp"

#include <memory>

#include "tsge/actions/card_effect_legal_move_generator.hpp"
#include "tsge/actions/command.hpp"

std::vector<CommandPtr> DeStalinizationRemoveMove::toCommand(
    const std::unique_ptr<Card>& /*card*/, const Board& /*board*/) const {
  std::vector<CommandPtr> commands;

  // 1. USSR影響力を除去
  commands.emplace_back(
      std::make_shared<RemoveInfluenceCommand>(Side::USSR, targetCountries_));

  // 2. 除去合計を算出
  int total_removed = 0;
  for (const auto& [country, amount] : targetCountries_) {
    total_removed += amount;
  }

  // 3. 除去数に応じた配置Requestを発行
  CardSpecialPlaceInfluenceConfig config;
  config.totalInfluence = total_removed;
  config.maxPerCountry = 2;
  config.allowedRegions = std::nullopt;
  config.excludeOpponentControlled = true;
  config.onlyEmptyCountries = false;

  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR, [card_enum = getCard(), config](const Board& board) {
        return CardEffectLegalMoveGenerator::
            generateCardSpecificPlaceInfluenceMoves(board, Side::USSR,
                                                    card_enum, config);
      }));

  return commands;
}
