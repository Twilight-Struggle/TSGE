#pragma once

#include <array>
#include <memory>

#include "tsge/core/board.hpp"
#include "tsge/core/phase_machine.hpp"
#include "tsge/game_state/card.hpp"
#include "tsge/players/player.hpp"
#include "tsge/players/policies.hpp"

class Game {
 public:
#ifdef TEST
  Game()
      : Game(Player<TestPolicy>{}, Player<TestPolicy>{}, defaultCardPool()) {}
  Game(Player<TestPolicy>&& player1, Player<TestPolicy>&& player2)
      : Game(std::move(player1), std::move(player2), defaultCardPool()) {}
#endif
  Game(Player<TestPolicy>&& player1, Player<TestPolicy>&& player2,
       const std::array<std::unique_ptr<Card>, 111>& cardpool);

  Board& getBoard() { return board_; }

  void next();

 private:
#ifdef TEST
  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }
#endif
  Board board_;

  std::array<Player<TestPolicy>, 2> players_;
  static void mayFail(bool success, const std::string& message);
};