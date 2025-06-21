#pragma once
#include <memory>

#include "tsge/core/board.hpp"

class Move;
class Board;

template <typename DecisionPolicy>
class Player {
 public:
  std::unique_ptr<Move> decideMove(
      const Board& board,
      const std::vector<std::unique_ptr<Move>>& legalMoves) {
    return decision_policy_.decideMove(board, legalMoves);
  }

 private:
  DecisionPolicy decision_policy_;
};