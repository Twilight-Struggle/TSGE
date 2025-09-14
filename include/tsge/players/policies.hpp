#pragma once
#include <memory>  // std::unique_ptr を使用するために追加
#include <vector>

#include "tsge/actions/move.hpp"

class Board;

class TestPolicy {
 public:
  explicit TestPolicy(std::vector<std::unique_ptr<Move>>&& moves);
  TestPolicy() = default;
  std::shared_ptr<Move> decideMove(
      const Board&, const std::vector<std::shared_ptr<Move>>& legalMoves,
      Side side);

 private:
  std::vector<std::unique_ptr<Move>> moves_;
  int currentMoveIndex_ = 0;
};