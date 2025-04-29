#pragma once
#include <vector>

#include "move.hpp"

class Game;

class TestPolicy {
 public:
  TestPolicy(std::vector<Move>& moves);
  TestPolicy() = default;
  Move decideMove(const Game& game);

 private:
  std::vector<Move> moves_;
  int currentMoveIndex_ = 0;
};