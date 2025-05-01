#pragma once
#include <memory>

class Move;
class Game;

template <typename DecisionPolicy>
class Player {
 public:
  std::unique_ptr<Move> decideMove(const Game& game) {
    return decision_policy_.decideMove(game);
  }

 private:
  DecisionPolicy decision_policy_;
};