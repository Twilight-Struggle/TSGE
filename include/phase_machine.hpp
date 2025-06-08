#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "board.hpp"
#include "move.hpp"

class PhaseMachine {
 public:
  static std::pair<std::vector<std::unique_ptr<Move>>, Side> step(
      Board& board,
      std::optional<std::unique_ptr<Move>>&& answer = std::nullopt);
};