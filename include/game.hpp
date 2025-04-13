#pragma once

#include "trackers.hpp"
#include "world_map.hpp"

class Game {
 public:
  Game();
  WorldMap& getWorldMap() { return worldMap_; }
  SpaceTrack& getSpaceTrack() { return spaceTrack_; }

 private:
  WorldMap worldMap_;
  SpaceTrack spaceTrack_;
};