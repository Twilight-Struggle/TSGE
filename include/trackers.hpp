#pragma once
#include <array>

#include "game_enums.hpp"

class SpaceTrack {
 public:
  SpaceTrack() = default;
  bool advanceSpaceTrack(Side side, int num);
  bool effectEnabled(Side side, int num);
  bool canSpace(Side side);
  void spaceTried(Side side) { spaceTried_[static_cast<std::size_t>(side)]++; }
  int getRollMax(Side side) const;

 private:
  std::array<int, 2> spaceTrack_ = {0, 0};
  std::array<int, 2> spaceTried_ = {0, 0};
  static constexpr std::array<std::array<int, 2>, 8> spaceVps_ = {
      {{2, 1}, {0, 0}, {2, 0}, {0, 0}, {3, 1}, {0, 0}, {4, 2}, {2, 0}}};
  static constexpr std::array<int, 8> rollMax_ = {3, 4, 3, 4, 3, 4, 3, 2};
};