#include "cards.hpp"

#include "board.hpp"
#include "game_enums.hpp"

bool DuckAndCover::event(Board& board, Side side) {
  board.getDefconTrack().changeDefcon(-1, board);
  board.changeVp(getVpMultiplier(Side::USA) *
                 (5 - board.getDefconTrack().getDefcon()));
  return true;
}

bool Fidel::event(Board& board, Side side) {
  auto& cuba = board.getWorldMap().getCountry(CountryEnum::CUBA);
  cuba.clearInfluence(Side::USA);
  auto sufficientInfluence =
      cuba.getStability() - cuba.getInfluence(Side::USSR);
  cuba.addInfluence(Side::USSR, std::max(0, sufficientInfluence));
  return true;
}

bool NuclearTestBan::event(Board& board, Side side) {
  board.changeVp((board.getDefconTrack().getDefcon() - 2) *
                 getVpMultiplier(side));
  board.getDefconTrack().changeDefcon(2, board);
  return true;
}
