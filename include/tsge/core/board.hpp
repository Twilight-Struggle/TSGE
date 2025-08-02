#pragma once

#include <array>
#include <variant>
#include <vector>

#include "tsge/actions/command.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/deck.hpp"
#include "tsge/game_state/trackers.hpp"
#include "tsge/game_state/world_map.hpp"
#include "tsge/utils/randomizer.hpp"

class Board {
 public:
  Board(const std::array<std::unique_ptr<Card>, 111>& cardpool)
      : cardpool_{cardpool}, deck_{randomizer_, cardpool_} {
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
  Randomizer& getRandomizer() { return randomizer_; }
  Deck& getDeck() { return deck_; }
  std::vector<CardEnum>& getPlayerHand(Side side) {
    return playerHands_[static_cast<size_t>(side)];
  }
  [[nodiscard]]
  const WorldMap& getWorldMap() const {
    return worldMap_;
  }
  [[nodiscard]]
  const SpaceTrack& getSpaceTrack() const {
    return spaceTrack_;
  }
  [[nodiscard]]
  const DefconTrack& getDefconTrack() const {
    return defconTrack_;
  }
  [[nodiscard]]
  const Randomizer& getRandomizer() const {
    return randomizer_;
  }
  [[nodiscard]]
  const Deck& getDeck() const {
    return deck_;
  }
  [[nodiscard]]
  const std::vector<CardEnum>& getPlayerHand(Side side) const {
    return playerHands_[static_cast<size_t>(side)];
  }
  [[nodiscard]]
  int getVp() const {
    return vp_;
  }
  [[nodiscard]]
  Side getCurrentArPlayer() const {
    return currentArPlayer_;
  }

  void pushState(std::variant<StateType, CommandPtr>&& state) {
    states_.emplace_back(std::move(state));
  }
  void changeVp(int delta) { vp_ += delta; }
  void setCurrentArPlayer(Side side) { currentArPlayer_ = side; }

  [[nodiscard]]
  std::array<int, 2> calculateDrawCount(int turn) const;
  void drawCardsForPlayers(int ussrDrawCount, int usaDrawCount);

  [[nodiscard]]
  CardEnum getHeadlineCard(Side side) const {
    return headlineCards_[static_cast<size_t>(side)];
  }
  void setHeadlineCard(Side side, CardEnum card) {
    headlineCards_[static_cast<size_t>(side)] = card;
  }
  void clearHeadlineCards() {
    headlineCards_[static_cast<size_t>(Side::USSR)] = CardEnum::Dummy;
    headlineCards_[static_cast<size_t>(Side::USA)] = CardEnum::Dummy;
  }
  [[nodiscard]]
  bool isHeadlineCardVisible(Side viewer, Side target) const;

#ifdef TEST
  void addCardToHand(Side side, CardEnum card) {
    playerHands_[static_cast<size_t>(side)].push_back(card);
  }
  void clearHand(Side side) { playerHands_[static_cast<size_t>(side)].clear(); }
#endif

 private:
  const std::array<std::unique_ptr<Card>, 111>& cardpool_;
  std::vector<std::variant<StateType, CommandPtr>> states_;
  WorldMap worldMap_;
  SpaceTrack spaceTrack_;
  DefconTrack defconTrack_;
  MilopsTrack milopsTrack_;
  TurnTrack turnTrack_;
  ActionRoundTrack actionRoundTrack_;
  Randomizer randomizer_;
  Deck deck_;
  std::array<std::vector<CardEnum>, 2> playerHands_;
  std::array<CardEnum, 2> headlineCards_ = {CardEnum::Dummy, CardEnum::Dummy};
  int vp_ = 0;
  Side currentArPlayer_ = Side::NEUTRAL;
};
