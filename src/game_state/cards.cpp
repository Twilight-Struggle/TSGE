#include "tsge/game_state/cards.hpp"

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"

bool DuckAndCover::DefconBasedVpChangeCommand::apply(Board& board) const {
  int current_defcon = board.getDefconTrack().getDefcon();
  int vp_change = (5 - current_defcon);
  board.pushState(std::make_unique<ChangeVpCommand>(Side::USA, vp_change));
  return true;
}

std::vector<CommandPtr> DuckAndCover::event(Side side) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_unique<ChangeDefconCommand>(-1));
  commands.push_back(std::make_unique<DefconBasedVpChangeCommand>(side));
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

bool NuclearTestBan::DefconBasedVpChangeCommand::apply(Board& board) const {
  int current_defcon = board.getDefconTrack().getDefcon();
  int vp_change = (current_defcon - 2);
  board.pushState(std::make_unique<ChangeVpCommand>(side_, vp_change));
  return true;
}

std::vector<CommandPtr> NuclearTestBan::event(Side side) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_unique<DefconBasedVpChangeCommand>(side));
  commands.push_back(std::make_unique<ChangeDefconCommand>(2));
  return commands;
}
