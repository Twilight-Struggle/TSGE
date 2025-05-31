#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "board.hpp"
#include "move.hpp"

class PhaseMachine {
 public:
  static std::pair<std::vector<std::unique_ptr<Move>>, Side> step(
      Board& board, const std::array<std::unique_ptr<Card>, 111>& cardpool_,
      std::optional<std::unique_ptr<Move>>&& answer = std::nullopt);
  static std::vector<std::unique_ptr<Move>> legalPlayerMoves(const Board& board,
                                                             Side side);
};