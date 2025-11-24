// どこで: src/game_state/cards/scoring_cards.cpp
// 何を: スコアリングカードのイベント実装
// なぜ: カード実装を分類し、可読性と保守性を向上させるため
#include "tsge/game_state/cards/scoring_cards.hpp"

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/core/board.hpp"

std::vector<CommandPtr> RegionScoringCard::event(Side /*side*/,
                                                 const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<ScoreRegionCommand>(region_));
  return commands;
}

bool RegionScoringCard::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> SoutheastAsiaScoring::event(
    Side /*side*/, const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<SoutheastAsiaScoringCommand>());
  return commands;
}

bool SoutheastAsiaScoring::canEvent(const Board& /*board*/) const {
  return true;
}
