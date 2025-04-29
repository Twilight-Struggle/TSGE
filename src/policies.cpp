#include "policies.hpp"

#include <stdexcept>

#include "game.hpp"

TestPolicy::TestPolicy(std::vector<Move>& moves) : moves_{std::move(moves)} {}

Move TestPolicy::decideMove(const Game& game) {
  if (currentMoveIndex_ >= moves_.size()) {
    throw std::runtime_error("No more moves available");
  }
  return moves_[currentMoveIndex_++];
}