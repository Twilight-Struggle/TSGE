#include "move.hpp"

#include <memory>

#include "action.hpp"
#include "game.hpp"

std::unique_ptr<const Action> PlaceInfluenceMove::toAction(const Game& game,
                                                           Side side) const {
  auto& card = game.getCardpool()[static_cast<int>(getCard())];
  return std::make_unique<PlaceInfluence>(side, card->getOps(),
                                          targetCountries_);
}

std::unique_ptr<const Action> CoupMove::toAction(const Game& game,
                                                 Side side) const {
  auto& card = game.getCardpool()[static_cast<int>(getCard())];
  return std::make_unique<Coup>(side, card->getOps(), targetCountry_);
}

std::unique_ptr<const Action> SpaceRaceMove::toAction(const Game& game,
                                                      Side side) const {
  auto& card = game.getCardpool()[static_cast<int>(getCard())];
  return std::make_unique<SpaceRace>(side, card->getOps());
}