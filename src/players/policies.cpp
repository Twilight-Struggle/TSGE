#include "tsge/players/policies.hpp"

#include <stdexcept>

TestPolicy::TestPolicy(std::vector<std::unique_ptr<Move>>&& moves)
    : moves_{std::move(moves)} {}

std::unique_ptr<Move> TestPolicy::decideMove(
    const Board& /*board*/,
    const std::vector<std::unique_ptr<Move>>& /*legal_moves*/) {
  if (currentMoveIndex_ >= moves_.size()) {
    throw std::runtime_error("No more moves available");
  }
  return std::move(moves_[currentMoveIndex_++]);
}