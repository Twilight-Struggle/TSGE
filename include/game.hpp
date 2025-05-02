#pragma once

#include <array>
#include <memory>
#include <stack>

#include "card.hpp"
#include "game_enums.hpp"
#include "player.hpp"
#include "policies.hpp"
#include "trackers.hpp"
#include "world_map.hpp"

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
  WorldMap& getWorldMap() { return worldMap_; }
  SpaceTrack& getSpaceTrack() { return spaceTrack_; }
  DefconTrack& getDefconTrack() { return defconTrack_; }
  MilopsTrack& getMilopsTrack() { return milopsTrack_; }
  TurnTrack& getTurnTrack() { return turnTrack_; }
  ActionRoundTrack& getActionRoundTrack() { return actionRoundTrack_; }
  int getVp() const { return vp_; }
  void changeVp(int delta);

  void next();

 private:
#ifdef TEST
  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }
#endif

  WorldMap worldMap_;
  SpaceTrack spaceTrack_;
  DefconTrack defconTrack_;
  MilopsTrack milopsTrack_;
  TurnTrack turnTrack_;
  ActionRoundTrack actionRoundTrack_;
  int vp_ = 0;

  std::stack<StateType> states_;
  std::array<Player<TestPolicy>, 2> players_;
  const std::array<std::unique_ptr<Card>, 111>& cardpool_;
  void actionExecute(Side side);
};