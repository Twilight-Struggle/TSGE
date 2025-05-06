#include <memory>
#include <optional>
#include <vector>

#include "board.hpp"
#include "move.hpp"

class PhaseMachine {
 public:
  static std::vector<Move> step(
      Board& board,
      std::optional<std::vector<std::unique_ptr<Move>>> answer = std::nullopt);
};