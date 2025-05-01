#pragma once
#include <memory>  // std::unique_ptr を使用するために追加
#include <vector>

#include "move.hpp"

class Game;

class TestPolicy {
 public:
  TestPolicy(std::vector<std::unique_ptr<Move>>&& moves);
  TestPolicy() = default;
  std::unique_ptr<Move> decideMove(const Game& game);

 private:
  std::vector<std::unique_ptr<Move>> moves_;
  int currentMoveIndex_ = 0;
};