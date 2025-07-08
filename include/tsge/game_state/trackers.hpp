#pragma once
#include <algorithm>
#include <array>

#include "tsge/enums/game_enums.hpp"

class Board;

class SpaceTrack {
 public:
  SpaceTrack() = default;
  void advanceSpaceTrack(Side side, int num) {
    spaceTrack_[static_cast<size_t>(side)] += num;
  }
  [[nodiscard]]
  bool effectEnabled(Side side, int num) const {
    return spaceTrack_[static_cast<std::size_t>(side)] >= num &&
           spaceTrack_[static_cast<std::size_t>(getOpponentSide(side))] < num;
  }
  [[nodiscard]]
  bool canSpaceChallenge(Side side) const {
    if (spaceTrack_[static_cast<std::size_t>(side)] == 8) [[unlikely]] {
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
  [[nodiscard]]
  bool canSpace(Side side, int opeValue) const {
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
  void spaceTried(Side side) { spaceTried_[static_cast<std::size_t>(side)]++; }
  [[nodiscard]]
  int getRollMax(Side side) const {
    const int space_pos = spaceTrack_[static_cast<std::size_t>(side)];
    // SpaceTrack position 8 has no rollMax (game ends), return 0
    if (space_pos >= 8) [[unlikely]] {
      return 0;
    }
    return ROLL_MAX[space_pos];
  }
  [[nodiscard]]
  int getSpaceTrackPosition(Side side) const {
    return spaceTrack_[static_cast<std::size_t>(side)];
  }
  static std::array<int, 2> getSpaceVp(int position) {
    return position > 0 && position <= 8 ? SPACE_VPS[position - 1]
                                         : std::array<int, 2>{0, 0};
  }

 private:
  std::array<int, 2> spaceTrack_ = {0, 0};
  std::array<int, 2> spaceTried_ = {0, 0};
  static constexpr std::array<std::array<int, 2>, 8> SPACE_VPS = {
      {{2, 1}, {0, 0}, {2, 0}, {0, 0}, {3, 1}, {0, 0}, {4, 2}, {2, 0}}};
  static constexpr std::array<int, 8> ROLL_MAX = {3, 4, 3, 4, 3, 4, 3, 2};
};

class DefconTrack {
 public:
  DefconTrack() = default;
  void setDefcon(int defcon) { defcon_ = defcon; }
  void changeDefcon(int delta) {
    int new_defcon = std::clamp(defcon_ + delta, 1, 5);
    defcon_ = new_defcon;
  }
  [[nodiscard]]
  int getDefcon() const {
    return defcon_;
  }

 private:
  int defcon_ = 5;
};

class MilopsTrack {
 public:
  MilopsTrack() = default;
  [[nodiscard]]
  int getMilops(Side side) const {
    return milopsTrack_[static_cast<std::size_t>(side)];
  }
  void resetMilopsTrack() {
    milopsTrack_[static_cast<std::size_t>(Side::USSR)] = 0;
    milopsTrack_[static_cast<std::size_t>(Side::USA)] = 0;
  }
  void advanceMilopsTrack(Side side, int num) {
    milopsTrack_[static_cast<std::size_t>(side)] +=
        std::min(num, 5 - milopsTrack_[static_cast<std::size_t>(side)]);
  }

 private:
  std::array<int, 2> milopsTrack_ = {0, 0};
};

class TurnTrack {
 public:
  TurnTrack() = default;
  [[nodiscard]]
  int getTurn() const {
    return turn_;
  }
  void nextTurn() {
    if (turn_ >= 10) [[unlikely]] {
    } else {
      turn_++;
    }
  }
  [[nodiscard]]
  int getDealedCards() const {
    return dealedCards_[turn_ - 1];
  }

 private:
  int turn_ = 1;
  std::array<int, 10> dealedCards_ = {8, 8, 8, 9, 9, 9, 9, 9, 9, 9};
};

class ActionRoundTrack {
 public:
  ActionRoundTrack() = default;
  [[nodiscard]]
  int getActionRound(Side side) const {
    return actionRound_[static_cast<std::size_t>(side)];
  }

 private:
  std::array<int, 2> actionRound_ = {0, 0};
  std::array<int, 10> actionRoundsByTurn_ = {6, 6, 6, 7, 7, 7, 7, 7, 7, 7};
};