#pragma once

#include "trackers.hpp"
#include "world_map.hpp"

class Game {
 public:
  Game();
  WorldMap& getWorldMap() { return worldMap_; }
  SpaceTrack& getSpaceTrack() { return spaceTrack_; }
  int getVp() const { return vp_; }
  void changeVp(int delta);

 private:
  WorldMap worldMap_;
  SpaceTrack spaceTrack_;
  int vp_ = 0;
};