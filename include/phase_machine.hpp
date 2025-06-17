#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "board.hpp"
#include "move.hpp"

class PhaseMachine {
 public:
  static std::tuple<std::vector<std::unique_ptr<Move>>, Side, std::optional<Side>> step(
      Board& board,
      std::optional<std::unique_ptr<Move>>&& answer = std::nullopt);
};