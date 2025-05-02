#include "game.hpp"

#ifndef TEST
#include <cstddef>
#include <random>
#endif
#include <stdexcept>

#include "game_enums.hpp"
#include "trackers.hpp"

Game::Game(Player<TestPolicy>&& player1, Player<TestPolicy>&& player2,
           const std::array<std::unique_ptr<Card>, 111>& cardpool)
    : worldMap_{WorldMap()},
      spaceTrack_{SpaceTrack()},
      defconTrack_{DefconTrack{*this}},
      milopsTrack_{MilopsTrack()},
      turnTrack_{TurnTrack()},
      actionRoundTrack_{ActionRoundTrack()},
      players_{{std::move(player1), std::move(player2)}},
      cardpool_{cardpool} {}

#ifndef TEST
Game::Game(Player<TestPolicy>&& player1, Player<TestPolicy>&& player2,
           const std::array<std::unique_ptr<Card>, 111>& cardpool)
    : worldMap_{WorldMap()},
      spaceTrack_{SpaceTrack()},
      defconTrack_{DefconTrack{*this}},
      milopsTrack_{MilopsTrack()},
      turnTrack_{TurnTrack()},
      actionRoundTrack_{ActionRoundTrack()},
      players_{{std::move(player1), std::move(player2)}},
      cardpool_{cardpool} {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> firstSecond(0, 1);
  if (firstSecond(gen) == 1) {
    std::swap(players_[0], players_[1]);
  }
}
#endif

void Game::changeVp(int delta) {
  vp_ += delta;
  if (vp_ <= -20 || vp_ >= 20) {
    // TODO:ゲーム終了
  }
}

void Game::next() {
  switch (states_.top()) {
    case StateType::AR_USSR:
      states_.pop();
      states_.push(StateType::AR_USA);
      break;
    case StateType::AR_USA:
      states_.pop();
      states_.push(StateType::AR_USSR);
      break;
    default:
      throw std::runtime_error("Invalid state");
  }
}

void Game::actionExecute(Side side) {
  auto& currentPlayer = players_[static_cast<int>(side)];
  auto input = currentPlayer.decideMove(*this);
}