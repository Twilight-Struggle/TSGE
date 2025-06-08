#include "game.hpp"

#include "game_enums.hpp"

#ifndef TEST
#include <cstddef>
#include <random>
#endif
#include <stdexcept>

#include "command.hpp"

#ifdef TEST
Game::Game(Player<TestPolicy>&& player1, Player<TestPolicy>&& player2,
           const std::array<std::unique_ptr<Card>, 111>& cardpool)
    : board_{Board(cardpool)},
      players_{{std::move(player1), std::move(player2)}} {}
#endif

#ifndef TEST
Game::Game(Player<TestPolicy>&& player1, Player<TestPolicy>&& player2,
           const std::array<std::unique_ptr<Card>, 111>& cardpool)
    : board_{Board(cardpool)},
      players_{{std::move(player1), std::move(player2)}} {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> firstSecond(0, 1);
  if (firstSecond(gen) == 1) {
    std::swap(players_[0], players_[1]);
  }
}
#endif

void Game::next() {
  std::optional<std::unique_ptr<Move>> pending;
  while (true) {
    auto [legalMoves, waitingForSide] =
        PhaseMachine::step(board_, std::move(pending));
    if (legalMoves.empty() && waitingForSide == Side::NEUTRAL) {
      // スタックが空→ゲーム終了
      break;
    }
    auto& player = players_[static_cast<size_t>(waitingForSide)];
    pending = player.decideMove(board_, legalMoves);
  }
}

void Game::mayFail(bool success, const std::string& message) {
  if (!success) {
    throw std::runtime_error("ここは失敗する可能性があるのでlogを出す");
  }
}