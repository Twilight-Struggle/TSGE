#include "move.hpp"

#include <memory>

#include "command.hpp"

std::unique_ptr<const Command> PlaceInfluenceMove::toCommand(
    const std::unique_ptr<Card>& card, Side side) const {
  return std::make_unique<PlaceInfluence>(side, card->getOps(),
                                          targetCountries_);
}

std::unique_ptr<const Command> CoupMove::toCommand(
    const std::unique_ptr<Card>& card, Side side) const {
  return std::make_unique<Coup>(side, card->getOps(), targetCountry_);
}

std::unique_ptr<const Command> SpaceRaceMove::toCommand(
    const std::unique_ptr<Card>& card, Side side) const {
  return std::make_unique<SpaceRace>(side, card->getOps());
}

std::unique_ptr<const Command> RealigmentMove::toCommand(
    const std::unique_ptr<Card>& card, Side side) const {
  return std::make_unique<Realigment>(side, targetCountry_);
}