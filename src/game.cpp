#include "game.hpp"

#include "game_enums.hpp"

#ifndef TEST
#include <cstddef>
#include <random>
#endif
#include <stdexcept>

#include "action.hpp"

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
      actionExecute(Side::USSR);
      break;
    case StateType::AR_USA:
      states_.pop();
      actionExecute(Side::USA);
      break;
    default:
      throw std::runtime_error("Invalid state");
  }
}

void Game::mayFail(bool success, const std::string& message) {
  if (!success) {
    throw std::runtime_error("ここは失敗する可能性があるのでlogを出す");
  }
}

void Game::actionExecute(Side side) {
  auto& currentPlayer = players_[static_cast<int>(side)];
  auto moveInput = currentPlayer.decideMove(*this);
  auto& card = getCardpool()[static_cast<int>(moveInput->getCard())];
  auto action = moveInput->toAction(card, side);
  // Eventの場合
  if (moveInput->getMoveType() == MoveType::EVENT) {
    mayFail(card->event(*this, side), "Event failed");
    if (card->getSide() == getOpponentSide(side)) {
      actionExecuteAfterEvent(side, card);
      states_.push(StateType::AR_COMPLETE);
    }
  }
  // それ以外
  else {
    // Realignment
    if (moveInput->getMoveType() == MoveType::REALIGNMENT) {
      mayFail(action->execute(*this), "Realignment failed");
      auto ops = card->getOps();
      for (int i = 0; i < ops - 1; ++i) {
        auto moveInput = currentPlayer.decideMove(*this);
        auto action = moveInput->toAction(card, side);
        mayFail(action->execute(*this), "Realignment failed");
      }
    } else {
      mayFail(action->execute(*this), "Action failed");
    }
    if (card->getSide() == getOpponentSide(side)) {
      // TODO:イベント発動条件を満たしていたら
      mayFail(card->event(*this, side), "Event failed");
    }
    states_.push(StateType::AR_COMPLETE);
  }
}

void Game::actionExecuteAfterEvent(Side side,
                                   const std::unique_ptr<Card>& card) {
  auto& currentPlayer = players_[static_cast<int>(side)];
  auto moveInput = currentPlayer.decideMove(*this);
  auto& card_in = getCardpool()[static_cast<int>(moveInput->getCard())];
  mayFail(card != card_in, "Card mismatch");
  auto action = moveInput->toAction(card, side);
  // Realignment
  if (moveInput->getMoveType() == MoveType::REALIGNMENT) {
    mayFail(action->execute(*this), "Realignment failed");
    auto ops = card->getOps();
    for (int i = 0; i < ops - 1; ++i) {
      auto moveInput = currentPlayer.decideMove(*this);
      auto action = moveInput->toAction(card, side);
      mayFail(action->execute(*this), "Realignment failed");
    }
  } else {
    mayFail(action->execute(*this), "Action failed");
  }
  states_.push(StateType::AR_COMPLETE);
}