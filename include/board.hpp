#pragma once

#include <variant>
#include <vector>

#include "command.hpp"
#include "game_enums.hpp"
#include "trackers.hpp"
#include "world_map.hpp"

class Board {
 public:
  Board()
      : worldMap_{},
        spaceTrack_{},
        defconTrack_{},
        milopsTrack_{},
        turnTrack_{},
        actionRoundTrack_{} {}
  std::vector<std::variant<StateType, CommandPtr>>& getStates() {
    return states_;
  }
  WorldMap& getWorldMap() { return worldMap_; }
  SpaceTrack& getSpaceTrack() { return spaceTrack_; }
  DefconTrack& getDefconTrack() { return defconTrack_; }
  MilopsTrack& getMilopsTrack() { return milopsTrack_; }
  TurnTrack& getTurnTrack() { return turnTrack_; }
  ActionRoundTrack& getActionRoundTrack() { return actionRoundTrack_; }
  int getVp() const { return vp_; }

  void pushState(std::variant<StateType, CommandPtr>&& state) {
    states_.emplace_back(state);
  }
  void changeVp(int delta) {
    vp_ += delta;
    if (vp_ <= -20 || vp_ >= 20) {
      // TODO:ゲーム終了
    }
  }

 private:
  std::vector<std::variant<StateType, CommandPtr>> states_;
  WorldMap worldMap_;
  SpaceTrack spaceTrack_;
  DefconTrack defconTrack_;
  MilopsTrack milopsTrack_;
  TurnTrack turnTrack_;
  ActionRoundTrack actionRoundTrack_;
  int vp_ = 0;
};
