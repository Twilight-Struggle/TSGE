#include "command.hpp"

#include "board.hpp"
#include "country.hpp"
#include "game_enums.hpp"
#include "randomizer.hpp"

bool ActionPlaceInfluence::apply(Board& board) const {
  for (const auto& targetCountry : targetCountries_) {
    board.getWorldMap()
        .getCountry(targetCountry.first)
        .addInfluence(side_, targetCountry.second);
  }
  return true;
}

bool ActionRealigment::apply(Board& board) const {
  // USSRかUSAならパス
  if (targetCountry_ == CountryEnum::USSR ||
      targetCountry_ == CountryEnum::USA) {
    return true;
  }
  auto& worldmap = board.getWorldMap();
  auto country = worldmap.getCountry(targetCountry_);

  auto ussr_dice = Randomizer::getInstance().rollDice();
  auto usa_dice = Randomizer::getInstance().rollDice();
  if (country.getInfluence(Side::USSR) > country.getInfluence(Side::USA)) {
    ussr_dice += 1;
  } else if (country.getInfluence(Side::USA) >
             country.getInfluence(Side::USSR)) {
    usa_dice += 1;
  }
  std::vector<Country> adjacentCountries;
  for (const auto& adjacentCountry : country.getAdjacentCountries()) {
    adjacentCountries.push_back(worldmap.getCountry(adjacentCountry));
  }
  for (const auto& adjacentCountry : adjacentCountries) {
    if (adjacentCountry.getControlSide() == Side::USSR) {
      ussr_dice += 1;
    } else if (adjacentCountry.getControlSide() == Side::USA) {
      usa_dice += 1;
    }
  }
  auto difference = ussr_dice - usa_dice;
  if (difference > 0) {
    country.removeInfluence(Side::USA, difference);
  } else if (difference < 0) {
    country.removeInfluence(Side::USSR, -difference);
  }
  return true;
}

bool ActionCoup::apply(Board& board) const {
  if (targetCountry_ == CountryEnum::USSR ||
      targetCountry_ == CountryEnum::USA) {
    return false;
  }
  auto& worldmap = board.getWorldMap();
  auto targetCountry = worldmap.getCountry(targetCountry_);
  if ((side_ == Side::USSR && targetCountry.getInfluence(Side::USA) == 0) ||
      (side_ == Side::USA && targetCountry.getInfluence(Side::USSR) == 0)) {
    return false;
  } else {
    auto coup_dice = Randomizer::getInstance().rollDice();
    coup_dice += card_->getOps();
    const auto defence_value = targetCountry.getStability() * 2;
    coup_dice -= defence_value;
    if (coup_dice < 0) {
      coup_dice = 0;
    }
    bool success = (coup_dice == 0) ? false : true;
    bool diff = targetCountry.getInfluence(getOpponentSide(side_)) - coup_dice;
    if (diff > 0) {
      targetCountry.removeInfluence(getOpponentSide(side_), coup_dice);
    } else {
      targetCountry.clearInfluence(getOpponentSide(side_));
      targetCountry.addInfluence(side_, -diff);
    }
    return true;
  }
}

bool ActionSpaceRace::apply(Board& board) const {
  auto& spaceTrack = board.getSpaceTrack();
  if (spaceTrack.canSpace(side_, card_->getOps())) {
    auto roll = Randomizer::getInstance().rollDice();
    if (roll <= spaceTrack.getRollMax(side_)) {
      spaceTrack.advanceSpaceTrack(board, side_, 1);
    }
    spaceTrack.spaceTried(side_);
    return true;
  } else {
    return false;
  }
}