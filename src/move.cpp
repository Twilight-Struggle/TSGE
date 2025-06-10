#include "move.hpp"

#include <memory>

#include "command.hpp"
#include "game_enums.hpp"

std::vector<CommandPtr> ActionPlaceInfluenceMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_unique<ActionPlaceInfluence>(
      getSide(), card, targetCountries_));
  if (getOpponentSide(getSide()) == card->getSide()) {
    // TODO 相手のイベントが発動 EventCommand push
  }
  return commands;
}

std::vector<CommandPtr> ActionCoupMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(
      std::make_unique<ActionCoup>(getSide(), card, targetCountry_));
  return commands;
}

std::vector<CommandPtr> ActionSpaceRaceMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_unique<ActionSpaceRace>(getSide(), card));
  return commands;
}

std::vector<CommandPtr> ActionRealigmentMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(
      std::make_unique<ActionRealigment>(getSide(), card, targetCountry_));
  return commands;
}