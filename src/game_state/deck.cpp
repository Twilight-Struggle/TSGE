#include "tsge/game_state/deck.hpp"

#include "tsge/game_state/card.hpp"
#include "tsge/utils/randomizer.hpp"

void Deck::reshuffleFromDiscard() {
  deck_.insert(deck_.end(), discardPile_.begin(), discardPile_.end());
  discardPile_.clear();
  randomizer_.shuffle(deck_);
}

void Deck::addCardsByWarPeriod(WarPeriod warPeriod) {
  std::vector<CardEnum> new_cards;

  for (size_t i = 0; i < cardpool_.size(); ++i) {
    if (cardpool_[i] && cardpool_[i]->getWarPeriod() == warPeriod) {
      auto card_enum = static_cast<CardEnum>(i);
      if (card_enum == CardEnum::CHINA_CARD) {
        continue;
      }
      new_cards.push_back(card_enum);
    }
  }

  deck_.insert(deck_.end(), new_cards.begin(), new_cards.end());
  randomizer_.shuffle(deck_);
}
