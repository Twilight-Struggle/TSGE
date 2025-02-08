#include "action.hpp"

#include "world_map.hpp"

void PlaceInfluence::execute(WorldMap& board, const Side side) {
  for (const auto& targetCountry : targetCountries_) {
    board.getCountry(targetCountry.first)
        .addInfluence(side, targetCountry.second);
  }
}