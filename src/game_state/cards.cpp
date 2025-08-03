#include "tsge/game_state/cards.hpp"

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"

std::vector<CommandPtr> DuckAndCover::event(Side side) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<ChangeDefconCommand>(-1));
  commands.push_back(std::make_shared<LambdaCommand>([](Board& board) {
    int current_defcon = board.getDefconTrack().getDefcon();
    int vp_change = (5 - current_defcon);
    board.pushState(std::make_shared<ChangeVpCommand>(Side::USA, vp_change));
  }));
  return commands;
}

bool DuckAndCover::canEvent(const Board& board) const {
  return true;
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

bool Fidel::canEvent(const Board& board) const {
  return true;
}

bool NuclearTestBan::canEvent(const Board& board) const {
  return true;
}

std::vector<CommandPtr> NuclearTestBan::event(Side side) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<LambdaCommand>([side](Board& board) {
    int current_defcon = board.getDefconTrack().getDefcon();
    int vp_change = (current_defcon - 2);
    board.pushState(std::make_shared<ChangeVpCommand>(side, vp_change));
  }));
  commands.push_back(std::make_shared<ChangeDefconCommand>(2));
  return commands;
}
