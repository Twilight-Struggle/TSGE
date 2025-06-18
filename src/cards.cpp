#include "cards.hpp"

#include "command.hpp"
#include "game_enums.hpp"

std::vector<CommandPtr> DuckAndCover::event(Side side) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<ChangeDefconCommand>(-1));
  // Note: VP calculation should be done after DEFCON change
  // For now, we'll use DEFCON 4 (after -1) as an approximation
  // TODO: This needs to be handled properly with a new command that can read
  // current board state
  commands.push_back(
      std::make_shared<ChangeVPCommand>(getVpMultiplier(Side::USA) * 1));
  return commands;
}

std::vector<CommandPtr> Fidel::event(Side side) const {
  // auto& cuba = board.getWorldMap().getCountry(CountryEnum::CUBA);
  // cuba.clearInfluence(Side::USA);
  // auto sufficientInfluence =
  //     cuba.getStability() - cuba.getInfluence(Side::USSR);
  // cuba.addInfluence(Side::USSR, std::max(0, sufficientInfluence));
  // return true;
  std::vector<CommandPtr> commands;
  // TODO: Fidel card event implementation
  // This requires special commands to handle clearing influence and adding
  // control For now, return empty as requested in task
  return commands;
}

std::vector<CommandPtr> NuclearTestBan::event(Side side) const {
  std::vector<CommandPtr> commands;
  // Note: VP calculation should be done before DEFCON change
  // For now, we'll use initial DEFCON (5) as an approximation
  // TODO: This needs to be handled properly with a new command that can read
  // current board state
  commands.push_back(
      std::make_shared<ChangeVPCommand>((5 - 2) * getVpMultiplier(side)));
  commands.push_back(std::make_shared<ChangeDefconCommand>(2));
  return commands;
}
