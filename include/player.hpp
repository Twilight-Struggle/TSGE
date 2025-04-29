#include "game.hpp"
#include "move.hpp"

template <typename DecisionPolicy>
class Player {
 public:
  Move decideMove(const Game& game) {
    return decision_policy_.decideMove(game);
  }

 private:
  DecisionPolicy decision_policy_;
};