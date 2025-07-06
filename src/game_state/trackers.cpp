#include "tsge/game_state/trackers.hpp"

#include <algorithm>

#include "tsge/enums/game_enums.hpp"

bool SpaceTrack::advanceSpaceTrack(Side side, int num) {
  spaceTrack_[static_cast<size_t>(side)] += num;
  return true;
}

bool SpaceTrack::effectEnabled(Side side, int num) const {
  return spaceTrack_[static_cast<std::size_t>(side)] >= num &&
         spaceTrack_[static_cast<std::size_t>(getOpponentSide(side))] < num;
}

bool SpaceTrack::canSpaceChallenge(Side side) const {
  if (spaceTrack_[static_cast<std::size_t>(side)] == 8) {
    return false;
  }
  if (spaceTried_[static_cast<std::size_t>(side)] == 2) {
    return false;
  }
  if (spaceTried_[static_cast<std::size_t>(side)] == 0) {
    return true;
  }
  return effectEnabled(side, 2);
}

bool SpaceTrack::canSpace(Side side, int opeValue) const {
  if (!canSpaceChallenge(side)) {
    return false;
  }
  auto space_index = spaceTrack_[static_cast<std::size_t>(side)];
  if (space_index == 7 && opeValue == 4) {
    return true;
  }
  if (4 <= space_index && space_index <= 6 && opeValue >= 3) {
    return true;
  }
  if (space_index <= 3 && opeValue >= 2) {
    return true;
  }
  return false;
}

int SpaceTrack::getRollMax(Side side) const {
  const int space_pos = spaceTrack_[static_cast<std::size_t>(side)];
  // SpaceTrack position 8 has no rollMax (game ends), return 0
  if (space_pos >= 8) [[unlikely]] {
    return 0;
  }
  return ROLL_MAX[space_pos];
}

bool DefconTrack::setDefcon(int defcon) {
  defcon_ = defcon;
  return true;
}

bool DefconTrack::changeDefcon(int delta) {
  int new_defcon = std::clamp(defcon_ + delta, 1, 5);
  defcon_ = new_defcon;
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
  if (turn_ >= 10) {
    return false;
  }
  turn_++;
  return true;
}