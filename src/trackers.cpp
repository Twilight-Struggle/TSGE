#include "trackers.hpp"

#include "game_enums.hpp"

bool SpaceTrack::advanceSpaceTrack(Side side, int num) {
  spaceTrack_[static_cast<int>(side)] += num;

  for (const auto& i : {1, 3, 5, 7, 8}) {
    if (spaceTrack_[static_cast<std::size_t>(side)] == i) {
      if (spaceTrack_[static_cast<std::size_t>(getOpponentSide(side))] < i) {
        // 得点計算有利
      } else {
        // 得点計算不利
      }
    }
    // 8に到達した場合そのターンのARを増やす
  }

  return true;
}

bool SpaceTrack::effectEnabled(Side side, int num) {
  return spaceTrack_[static_cast<std::size_t>(side)] >= num &&
         spaceTrack_[static_cast<std::size_t>(getOpponentSide(side))] < num;
}

bool SpaceTrack::canSpace(Side side) {
  if (spaceTried_[static_cast<std::size_t>(side)] == 2)
    return false;
  else if (spaceTried_[static_cast<std::size_t>(side)] == 0)
    return true;
  else
    return effectEnabled(side, 2);
}