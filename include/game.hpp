#pragma once

#include <array>
#include <memory>
#include <stack>

#include "board.hpp"
#include "card.hpp"
#include "game_enums.hpp"
#include "player.hpp"
#include "policies.hpp"

class Game {
 public:
#ifdef TEST
  Game()
      : Game(Player<TestPolicy>{}, Player<TestPolicy>{}, defaultCardPool()) {}
  Game(Player<TestPolicy>&& p1, Player<TestPolicy>&& p2)
      : Game(std::move(p1), std::move(p2), defaultCardPool()) {}
#endif
  Game(Player<TestPolicy>&& player1, Player<TestPolicy>&& player2,
       const std::array<std::unique_ptr<Card>, 111>& cardpool);
  const std::array<std::unique_ptr<Card>, 111>& getCardpool() const {
    return cardpool_;
  }
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

  std::stack<StateType> states_;
  std::array<Player<TestPolicy>, 2> players_;
  const std::array<std::unique_ptr<Card>, 111>& cardpool_;
  void mayFail(bool success, const std::string& message);
  void actionExecute(Side side);
  void actionExecuteAfterEvent(Side side, const std::unique_ptr<Card>& card);
};