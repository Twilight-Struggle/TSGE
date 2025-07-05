#pragma once
#include <array>

#include "tsge/enums/game_enums.hpp"

class Board;

class SpaceTrack {
 public:
  SpaceTrack() = default;
  bool advanceSpaceTrack(Side side, int num);
  bool effectEnabled(Side side, int num) const;
  bool canSpaceChallenge(Side side) const;
  bool canSpace(Side side, int opeValue) const;
  void spaceTried(Side side) { spaceTried_[static_cast<std::size_t>(side)]++; }
  int getRollMax(Side side) const;
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
  bool setDefcon(int defcon);
  bool changeDefcon(int delta);
  int getDefcon() const { return defcon_; }

 private:
  int defcon_ = 5;
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

class TurnTrack {
 public:
  TurnTrack() = default;
  int getTurn() const { return turn_; }
  bool nextTurn();
  int getDealedCards() const { return dealedCards_[turn_ - 1]; }

 private:
  int turn_ = 1;
  std::array<int, 10> dealedCards_ = {8, 8, 8, 9, 9, 9, 9, 9, 9, 9};
};

class ActionRoundTrack {
 public:
  ActionRoundTrack() = default;
  int getActionRound(Side side) const {
    return actionRound_[static_cast<std::size_t>(side)];
  }

 private:
  std::array<int, 2> actionRound_ = {0, 0};
  std::array<int, 10> actionRoundsByTurn_ = {6, 6, 6, 7, 7, 7, 7, 7, 7, 7};
};