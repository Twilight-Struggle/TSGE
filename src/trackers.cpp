#include "trackers.hpp"

#include "game_enums.hpp"

bool SpaceTrack::advanceSpaceTrack(Side side, int num) {
  // TODO:将来的にはGameインスタンスを引数に取る(得点変更のため)
  spaceTrack_[static_cast<int>(side)] += num;

  for (const auto& i : {1, 3, 5, 7, 8}) {
    if (spaceTrack_[static_cast<std::size_t>(side)] == i) {
      if (spaceTrack_[static_cast<std::size_t>(getOpponentSide(side))] < i) {
        // TODO:得点計算有利
      } else {
        // TODO:得点計算不利
      }
    }
    // TODO:8に到達した場合そのターンのARを増やす
  }

  return true;
}

bool SpaceTrack::effectEnabled(Side side, int num) const {
  return spaceTrack_[static_cast<std::size_t>(side)] >= num &&
         spaceTrack_[static_cast<std::size_t>(getOpponentSide(side))] < num;
}

bool SpaceTrack::canSpaceChallenge(Side side) const {
  if (spaceTrack_[static_cast<std::size_t>(side)] == 8) return false;
  if (spaceTried_[static_cast<std::size_t>(side)] == 2)
    return false;
  else if (spaceTried_[static_cast<std::size_t>(side)] == 0)
    return true;
  else
    return effectEnabled(side, 2);
}

bool SpaceTrack::canSpace(Side side, int opeValue) const {
  if (!canSpaceChallenge(side)) return false;
  auto spaceIndex = spaceTrack_[static_cast<std::size_t>(side)];
  if (spaceIndex == 7 && opeValue == 4)
    return true;
  else if (4 <= spaceIndex && spaceIndex <= 6 && opeValue >= 3)
    return true;
  else if (spaceIndex <= 3 && opeValue >= 2)
    return true;
  else
    return false;
}

int SpaceTrack::getRollMax(Side side) const {
  return rollMax_[spaceTrack_[static_cast<std::size_t>(side)]];
}