#pragma once

#include <array>
#include <variant>
#include <vector>

#include "tsge/actions/command.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/trackers.hpp"
#include "tsge/game_state/world_map.hpp"

class Board {
 public:
  Board(const std::array<std::unique_ptr<Card>, 111>& cardpool)
      : cardpool_{cardpool} {
    for (auto& hand : playerHands_) {
      hand.reserve(9);
    }
  }
  [[nodiscard]]
  const std::array<std::unique_ptr<Card>, 111>& getCardpool() const {
    return cardpool_;
  }
  std::vector<std::variant<StateType, CommandPtr>>& getStates() {
    return states_;
  }
  WorldMap& getWorldMap() { return worldMap_; }
  SpaceTrack& getSpaceTrack() { return spaceTrack_; }
  DefconTrack& getDefconTrack() { return defconTrack_; }
  MilopsTrack& getMilopsTrack() { return milopsTrack_; }
  TurnTrack& getTurnTrack() { return turnTrack_; }
  ActionRoundTrack& getActionRoundTrack() { return actionRoundTrack_; }
  std::vector<CardEnum>& getPlayerHand(Side side) {
    return playerHands_[static_cast<size_t>(side)];
  }
  [[nodiscard]]
  const WorldMap& getWorldMap() const {
    return worldMap_;
  }
  [[nodiscard]]
  const std::vector<CardEnum>& getPlayerHand(Side side) const {
    return playerHands_[static_cast<size_t>(side)];
  }
  [[nodiscard]]
  int getVp() const {
    return vp_;
  }

  void pushState(std::variant<StateType, CommandPtr>&& state) {
    states_.emplace_back(std::move(state));
  }
  void changeVp(int delta) { vp_ += delta; }

 private:
  const std::array<std::unique_ptr<Card>, 111>& cardpool_;
  std::vector<std::variant<StateType, CommandPtr>> states_;
  WorldMap worldMap_;
  SpaceTrack spaceTrack_;
  DefconTrack defconTrack_;
  MilopsTrack milopsTrack_;
  TurnTrack turnTrack_;
  ActionRoundTrack actionRoundTrack_;
  std::array<std::vector<CardEnum>, 2> playerHands_;
  int vp_ = 0;
};
