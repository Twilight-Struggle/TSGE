// どこで: src/game_state/cards/special_cards.cpp
// 何を: 特殊なカードのイベント実装
// なぜ: カード実装を分類し、可読性と保守性を向上させるため
#include "tsge/game_state/cards/special_cards.hpp"

#include <map>
#include <memory>

#include "tsge/actions/card_effect_legal_move_generator.hpp"
#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"

namespace {

std::vector<std::shared_ptr<Move>> buildDeStalinizationRemoveMoves(
    const Board& board, Side side) {
  const auto& world_map = board.getWorldMap();
  bool has_candidate = false;
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);
    if (world_map.getCountry(country_enum).getInfluence(Side::USSR) > 0) {
      has_candidate = true;
      break;
    }
  }

  if (!has_candidate) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  const CardEnum card_enum = CardEnum::DE_STALINIZATION;
  results.emplace_back(std::make_shared<DeStalinizationRemoveMove>(
      card_enum, side, std::map<CountryEnum, int>{}));

  for (int total_remove = 1; total_remove <= 4; ++total_remove) {
    auto patterns =
        CardEffectLegalMoveGenerator::enumerateRemoveInfluencePatterns(
            board, Side::USSR, total_remove, /*maxPerCountry=*/0, std::nullopt,
            std::nullopt, RemovalSaturationStrategy::kRequireExact);
    for (const auto& pattern : patterns) {
      results.emplace_back(std::make_shared<DeStalinizationRemoveMove>(
          card_enum, side, pattern));
    }
  }

  return results;
}

}  // namespace

void registerDeStalinizationCardEffectGenerator() {
  CardEffectLegalMoveGenerator::registerGenerator(
      CardEnum::DE_STALINIZATION, [](const Board& board, Side side) {
        return buildDeStalinizationRemoveMoves(board, side);
      });
}

std::vector<CommandPtr> DeStainization::event(Side side,
                                              const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  // De-Stalinization: USSR影響力を1-4除去し、除去した数だけ配置
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR, [](const Board& board) -> std::vector<std::shared_ptr<Move>> {
        return CardEffectLegalMoveGenerator::generate(
            CardEnum::DE_STALINIZATION, board, Side::USSR);
      }));
  return commands;
}

bool DeStainization::canEvent(const Board& /*board*/) const {
  return true;
}
