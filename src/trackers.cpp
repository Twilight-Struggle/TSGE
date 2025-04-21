#include "trackers.hpp"

#include "game.hpp"
#include "game_enums.hpp"

bool SpaceTrack::advanceSpaceTrack(Game& game, Side side, int num) {
  spaceTrack_[static_cast<int>(side)] += num;

  for (const auto& i : {1, 3, 5, 7, 8}) {
    if (spaceTrack_[static_cast<std::size_t>(side)] == i) {
      if (spaceTrack_[static_cast<std::size_t>(getOpponentSide(side))] < i) {
        // 得点計算有利
        game.changeVp(spaceVps_[i - 1][0] * getVpMultiplier(side));
      } else {
        // 得点計算不利
        game.changeVp(spaceVps_[i - 1][1] * getVpMultiplier(side));
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

DefconTrack::DefconTrack(Game& game) : game_{game} {}

bool DefconTrack::setDefcon(int defcon) {
  if (defcon < 1 || defcon > 5) return false;
  auto defconChanged = defcon_ != defcon;
  defcon_ = defcon;
  if (defcon_ < 2) {
    // TODO:ゲーム終了
  }
  if (defconChanged && defcon_ == 2) {
    // TODO:NORADの効果を適用
  }
  return true;
}

bool DefconTrack::changeDefcon(int delta) {
  auto newDefcon = defcon_ + std::min(delta, 5 - defcon_);
  auto defconChanged = defcon_ != newDefcon;
  defcon_ = newDefcon;
  if (defcon_ < 2) {
    // TODO:ゲーム終了
  }
  if (defconChanged && defcon_ == 2) {
    // TODO:NORADの効果を適用
  }
  return true;
}