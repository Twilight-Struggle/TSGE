#pragma once

#include <array>
#include <memory>
#include <vector>

#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"

class Card;
class Randomizer;

class Deck {
 public:
  Deck(Randomizer& randomizer,
       const std::array<std::unique_ptr<Card>, 111>& cardpool)
      : randomizer_{randomizer}, cardpool_{cardpool} {
    deck_.reserve(111);
    discardPile_.reserve(111);
    removedCards_.reserve(111);
  }

  void reshuffleFromDiscard();
  void addEarlyWarCards() { addCardsByWarPeriod(WarPeriod::EARLY_WAR); }
  void addMidWarCards() { addCardsByWarPeriod(WarPeriod::MID_WAR); }
  void addLateWarCards() { addCardsByWarPeriod(WarPeriod::LATE_WAR); }

  [[nodiscard]]
  const std::vector<CardEnum>& getDeck() const {
    return deck_;
  }
  [[nodiscard]]
  const std::vector<CardEnum>& getDiscardPile() const {
    return discardPile_;
  }
  [[nodiscard]]
  const std::vector<CardEnum>& getRemovedCards() const {
    return removedCards_;
  }

  std::vector<CardEnum>& getDeck() { return deck_; }
  std::vector<CardEnum>& getDiscardPile() { return discardPile_; }
  std::vector<CardEnum>& getRemovedCards() { return removedCards_; }

 private:
  void addCardsByWarPeriod(WarPeriod warPeriod);

  Randomizer& randomizer_;
  const std::array<std::unique_ptr<Card>, 111>& cardpool_;
  std::vector<CardEnum> deck_;
  std::vector<CardEnum> discardPile_;
  std::vector<CardEnum> removedCards_;
};