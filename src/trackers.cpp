#include "trackers.hpp"

#include <algorithm>

#include "game_enums.hpp"

bool SpaceTrack::advanceSpaceTrack(Side side, int num) {
  spaceTrack_[static_cast<int>(side)] += num;
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

bool DefconTrack::setDefcon(int defcon) {
  defcon_ = defcon;
  return true;
}

bool DefconTrack::changeDefcon(int delta) {
  int newDefcon = std::clamp(defcon_ + delta, 1, 5);
  defcon_ = newDefcon;
  return true;
}

bool MilopsTrack::resetMilopsTrack() {
  milopsTrack_[static_cast<std::size_t>(Side::USSR)] = 0;
  milopsTrack_[static_cast<std::size_t>(Side::USA)] = 0;
  return true;
}

bool MilopsTrack::advanceMilopsTrack(Side side, int num) {
  milopsTrack_[static_cast<std::size_t>(side)] +=
      std::min(num, 5 - milopsTrack_[static_cast<std::size_t>(side)]);
  return true;
}

bool TurnTrack::nextTurn() {
  if (turn_ >= 10) return false;
  turn_++;
  return true;
}