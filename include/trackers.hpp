#pragma once
#include <array>

#include "game_enums.hpp"

class Game;

class SpaceTrack {
 public:
  SpaceTrack() = default;
  bool advanceSpaceTrack(Game& game, Side side, int num);
  bool effectEnabled(Side side, int num) const;
  bool canSpaceChallenge(Side side) const;
  bool canSpace(Side side, int opeValue) const;
  void spaceTried(Side side) { spaceTried_[static_cast<std::size_t>(side)]++; }
  int getRollMax(Side side) const;

 private:
  std::array<int, 2> spaceTrack_ = {0, 0};
  std::array<int, 2> spaceTried_ = {0, 0};
  static constexpr std::array<std::array<int, 2>, 8> spaceVps_ = {
      {{2, 1}, {0, 0}, {2, 0}, {0, 0}, {3, 1}, {0, 0}, {4, 2}, {2, 0}}};
  static constexpr std::array<int, 8> rollMax_ = {3, 4, 3, 4, 3, 4, 3, 2};
};

class DefconTrack {
 public:
  DefconTrack(Game& game);
  bool setDefcon(int defcon);
  bool changeDefcon(int delta);
  int getDefcon() const { return defcon_; }

 private:
  int defcon_ = 5;
  Game& game_;
};

class MilopsTrack {
 public:
  MilopsTrack() = default;
  int getMilops(Side side) const {
    return milopsTrack_[static_cast<std::size_t>(side)];
  }
  bool resetMilopsTrack();
  bool advanceMilopsTrack(Side side, int num);

 private:
  std::array<int, 2> milopsTrack_ = {0, 0};
};
