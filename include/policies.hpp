#include "game.hpp"
#include "move.hpp"

class TestPolicy {
 public:
  Move decideMove(const Game& game);
};