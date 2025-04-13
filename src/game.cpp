#include "game.hpp"

#include "trackers.hpp"
#include "world_map.hpp"

Game::Game() : worldMap_{WorldMap()}, spaceTrack_{SpaceTrack()} {}