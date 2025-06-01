#include "move.hpp"

#include <memory>

#include "command.hpp"

CommandPtr ActionPlaceInfluenceMove::toCommand(
    const std::unique_ptr<Card>& card, Side side) const {
  return std::make_unique<ActionPlaceInfluence>(side, card, targetCountries_);
}

CommandPtr ActionCoupMove::toCommand(const std::unique_ptr<Card>& card,
                                     Side side) const {
  return std::make_unique<ActionCoup>(side, card, targetCountry_);
}

CommandPtr ActionSpaceRaceMove::toCommand(const std::unique_ptr<Card>& card,
                                          Side side) const {
  return std::make_unique<ActionSpaceRace>(side, card);
}

CommandPtr ActionRealigmentMove::toCommand(const std::unique_ptr<Card>& card,
                                           Side side) const {
  return std::make_unique<ActionRealigment>(side, card, targetCountry_);
}