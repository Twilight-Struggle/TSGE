#include "cards.hpp"

#include "game.hpp"
#include "game_enums.hpp"

bool DuckAndCover::event(Game& game, Side side) {
  game.getDefconTrack().changeDefcon(-1);
  game.changeVp(getVpMultiplier(Side::USA) *
                (5 - game.getDefconTrack().getDefcon()));
  return true;
}

bool Fidel::event(Game& game, Side side) {
  auto& cuba = game.getWorldMap().getCountry(CountryEnum::CUBA);
  cuba.clearInfluence(Side::USA);
  auto sufficientInfluence =
      cuba.getStability() - cuba.getInfluence(Side::USSR);
  cuba.addInfluence(Side::USSR, std::max(0, sufficientInfluence));
  return true;
}

bool NuclearTestBan::event(Game& game, Side side) {
  game.changeVp((game.getDefconTrack().getDefcon() - 2) *
                getVpMultiplier(side));
  game.getDefconTrack().changeDefcon(2);
  return true;
}
