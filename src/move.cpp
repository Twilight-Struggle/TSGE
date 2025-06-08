#include "move.hpp"

#include <memory>

#include "command.hpp"

CommandPtr ActionPlaceInfluenceMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  return std::make_unique<ActionPlaceInfluence>(getSide(), card,
                                                targetCountries_);
}

CommandPtr ActionCoupMove::toCommand(const std::unique_ptr<Card>& card) const {
  return std::make_unique<ActionCoup>(getSide(), card, targetCountry_);
}

CommandPtr ActionSpaceRaceMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  return std::make_unique<ActionSpaceRace>(getSide(), card);
}

CommandPtr ActionRealigmentMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  return std::make_unique<ActionRealigment>(getSide(), card, targetCountry_);
}