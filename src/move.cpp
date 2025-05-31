#include "move.hpp"

#include <memory>

#include "command.hpp"

CommandPtr PlaceInfluenceMove::toCommand(const std::unique_ptr<Card>& card,
                                         Side side) const {
  return std::make_unique<PlaceInfluence>(side, card->getOps(),
                                          targetCountries_);
}

CommandPtr CoupMove::toCommand(const std::unique_ptr<Card>& card,
                               Side side) const {
  return std::make_unique<Coup>(side, card->getOps(), targetCountry_);
}

CommandPtr SpaceRaceMove::toCommand(const std::unique_ptr<Card>& card,
                                    Side side) const {
  return std::make_unique<SpaceRace>(side, card->getOps());
}

CommandPtr RealigmentMove::toCommand(const std::unique_ptr<Card>& card,
                                     Side side) const {
  return std::make_unique<Realigment>(side, targetCountry_);
}