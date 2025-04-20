#include "game.hpp"

#include "trackers.hpp"
#include "world_map.hpp"

Game::Game() : worldMap_{WorldMap()}, spaceTrack_{SpaceTrack()} {}

void Game::changeVp(int delta) {
  vp_ += delta;
  if (vp_ <= -20 || vp_ >= 20) {
    // TODO:ゲーム終了
  }
}