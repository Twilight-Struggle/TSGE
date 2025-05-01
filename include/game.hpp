#pragma once

#include <stack>

#include "game_enums.hpp"
#include "player.hpp"
#include "policies.hpp"
#include "trackers.hpp"
#include "world_map.hpp"

class Game {
 public:
#ifdef TEST
  Game();
#endif
  Game(Player<TestPolicy>& player1, Player<TestPolicy>& player2);
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
  WorldMap worldMap_;
  SpaceTrack spaceTrack_;
  DefconTrack defconTrack_;
  MilopsTrack milopsTrack_;
  TurnTrack turnTrack_;
  ActionRoundTrack actionRoundTrack_;
  int vp_ = 0;

  std::stack<StateType> states_;
  std::array<Player<TestPolicy>, 2> players_;
  void actionExecute(Side side);
};