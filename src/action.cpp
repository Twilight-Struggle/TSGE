#include "action.hpp"

void PlaceInfluence::execute(Board& board, const Side side) {
  for (const auto& targetCountry : targetCountries_) {
    board.getCountry(targetCountry.first)
        .addInfluence(side, targetCountry.second);
  }
}