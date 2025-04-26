#include "cards.hpp"

#include "game.hpp"

bool NuclearTestBan::event(Game& game, Side side) {
  game.changeVp((game.getDefconTrack().getDefcon() - 2) *
                getVpMultiplier(side));
  game.getDefconTrack().changeDefcon(2);
  return true;
}
